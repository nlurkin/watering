#!/bin/env python
'''
Created on 87 Jun 2020

@author: Nicolas Lurkin
'''

import dash
import dash_html_components as html
import dash_bootstrap_components as dbc
import dash_core_components as dcc
from dash.dependencies import Input, Output, State, MATCH, ALL
import plotly.graph_objects as go
import pandas as pd
from waterapp import app, mongoClient
from mongodb import to_utc, from_utc
import datetime
import dateutil


def build_sensor_card(sensor):
    sensor_element = None
    if sensor["data-type"] == "string":
        sensor_element = [html.H5(sensor["display"]), dcc.Textarea(
          value = '',
          style = {'width': '100%', 'height': '300px'},
          id = {"type": "string_sensor", "sensor": sensor["sensor"]},
          readOnly = True,
          contentEditable = True,
          persistence = False,
          rows = 30,
        )]
    elif sensor["data-type"] == "bool":
        sensor_element = [dcc.Graph(
            id = {"type": "bool_sensor", "sensor": sensor["sensor"]},
            # config = {'displayModeBar': False},
            animate = True,
            )]
        if "controller" in sensor and sensor["controller"]:
            day = to_utc().strftime("%Y-%m-%d")
            controller_last_value = mongoClient.get_controller_values(sensor["sensor"], day)
            sensor_element.append(dbc.Checklist(options = [{"label": "", "value": 1}],
            value = [] if (controller_last_value is None or len(controller_last_value) == 0 or controller_last_value[-1]["val"] == 0) else [1], id = {"type": "bool_controller", "sensor": sensor["sensor"]}, switch = True,))
            sensor_element.append(html.P(id = {"type": "dummy", "sensor": sensor["sensor"]}))
    elif sensor["data-type"] == "float":
        sensor_element = [dcc.Graph(
            id = {"type": "float_sensor", "sensor": sensor["sensor"]},
            # config = {'displayModeBar': False},
            animate = True,
            )]

    return dbc.Card(sensor_element)


def generate_dashboard_column(col_description):
    col = []
    sensor_docs = mongoClient.get_sensor_by_id(col_description)
    for sensor in col_description:
        col.append(build_sensor_card(sensor_docs[sensor]))
    return col


def build_controls():
    now = from_utc()
    switches = dbc.FormGroup([
        dbc.Label("Controls"),
        dbc.Checklist(
            options = [
                {"label": "Autoscale", "value": 1},
            ],
            value = [1],
            id = "control_switches",
            switch = True,)],
        )
    date_range = dbc.FormGroup([dbc.Label("Data range"),
        dcc.DatePickerRange(id = "date_range",
                            display_format = "D/M/Y",
                            start_date = now - datetime.timedelta(days = 1),
                            end_date = now + datetime.timedelta(minutes = 10),
                            first_day_of_week = 1,
                            )],
        )
    update_interval = dbc.FormGroup([dbc.Label("Refresh rate [s]"),
        dcc.Input(type = "number", id = "refresh_rate", value = 10, debounce = True),
        ]
        )
    return dbc.Collapse(dbc.Row([dbc.Col(switches, width = 1), dbc.Col(date_range, width = 2), dbc.Col(update_interval, width = 1)]), id = "control_menu")


def generate_layout(dashboard_name):
    db_doc = mongoClient.get_dashboard_by_name(dashboard_name)
    left_col = generate_dashboard_column(db_doc["left"])
    right_col = generate_dashboard_column(db_doc["right"])

    return [dcc.Interval(id = 'interval-component', interval = 1 * 10000, n_intervals = 0),
            dbc.Row([dbc.Button("Controls", color = "link", id = "control_menu_btn")]),
            build_controls(),
            dbc.Row([dbc.Col(left_col, style = {"padding": "0px"}), dbc.Col(right_col, style = {"padding": "0px"})])
            ]


def get_and_merge_data(sensor_name, sd, ed):
    value_doc = mongoClient.get_all_sensor_values(sensor_name, dateutil.parser.parse(sd).strftime("%Y-%m-%d"), dateutil.parser.parse(ed).strftime("%Y-%m-%d"))
    sensor_doc = mongoClient.get_sensor_by_name(sensor_name)
    values = []
    for doc in value_doc:
        values.extend(doc["samples"])
    if len(values) == 0:
        value_doc = None
    else:
        value_doc = {'samples': values}

    with_setpoint = False
    if value_doc is None:
        df = pd.DataFrame({"val": []})
    else:
        if "samples" not in value_doc:
            df = pd.DataFrame({"val": []})
        else:
            df = pd.DataFrame(value_doc["samples"])
            df.index = pd.to_datetime(df['ts'], unit = "s", utc = True)
        if "setpoint" in value_doc:
            dsp = pd.DataFrame(value_doc["setpoint"])
            dsp.index = pd.to_datetime(dsp['ts'], unit = "s", utc = True)
            dsp = dsp.rename(columns = {"val": "sp"})
            df = pd.merge(df, dsp, how = 'outer', left_index = True, right_index = True)
            with_setpoint = True
        new_index = to_utc()
        df = df.append(pd.DataFrame(index = [new_index], data = df.tail(1).values, columns = df.columns))

    if len(df) > 0:
        df.index = df.index.tz_convert("Europe/Brussels")
        min_max = [df.index[0], df.index[-1]]
    else:
        min_max = [from_utc(), from_utc()]

    return df, sensor_doc, min_max, with_setpoint


@app.callback(
    Output("control_menu", "is_open"),
    [Input("control_menu_btn", "n_clicks")],
    [State("control_menu", "is_open")],
)
def toggle_collapse(n, is_open):
    if n:
        return not is_open
    return is_open


@app.callback(Output("interval-component", "interval"),
              Input("refresh_rate", "value"))
def update_interval(seconds):
    ctx = dash.callback_context
    if not ctx.triggered:
        return dash.no_update

    return seconds * 1000


@app.callback(Output({"type": "string_sensor", "sensor": MATCH}, 'value'),
              [Input('interval-component', 'n_intervals')],
              [State({"type": "string_sensor", "sensor": MATCH}, 'id'),
               State("control_switches", "value"),
               State("date_range", "start_date"),
               State("date_range", "end_date")
               ])
def update_string_metrics(_, sensor_name, control_switches, sd, ed):
    day = to_utc().strftime("%Y-%m-%d")
    value_doc = mongoClient.get_sensor_values(sensor_name["sensor"], sd, ed)
    if not value_doc:
        return ""
    return "".join([_["val"] for _ in value_doc["samples"]])


@app.callback(Output({"type": "bool_sensor", "sensor": MATCH}, 'figure'),
              [Input('interval-component', 'n_intervals')],
              [State({"type": "bool_sensor", "sensor": MATCH}, 'id'),
               State("control_switches", "value"),
               State("date_range", "start_date"),
               State("date_range", "end_date")
               ])
def update_bool_metrics(_, sensor_name, control_switches, sd, ed):
    df, sensor_doc, min_max, with_setpoint = get_and_merge_data(sensor_name["sensor"], sd, ed)

    figure = go.Figure().add_trace(go.Scatter(
                    x = df.index,
                    y = df["val"],
                    mode = "lines+markers",
                    name = "Read"
                    )
                )
    if with_setpoint:
        figure = figure.add_trace(go.Scatter(
                    x = df.index,
                    y = df["sp"],
                    mode = "lines+markers",
                    name = "Set",
                    connectgaps = True
                    )
        )

    set_style(figure, sensor_doc["display"], control_switches, min_max)
    return figure


@app.callback(Output({"type": "dummy", "sensor": MATCH}, 'value'),
              [Input({"type": "bool_controller", "sensor": MATCH}, 'value')],
              [State({"type": "bool_sensor", "sensor": MATCH}, 'id')])
def update_bool_controller_setpoint(controller_value, sensor_name):
    ctx = dash.callback_context
    if not ctx.triggered:
        return dash.no_update

    sensor_name = sensor_name["sensor"]

    sensor_doc = mongoClient.sensors_db.find_one({"sensor": sensor_name})

    val = 0 if len(controller_value) == 0 else 1
    mongoClient.update_controller_values(sensor_doc, sensor_name, val)

    return ""


@app.callback(Output({"type": "float_sensor", "sensor": MATCH}, 'figure'),
              [Input('interval-component', 'n_intervals')],
              [State({"type": "float_sensor", "sensor": MATCH}, 'id'),
               State("control_switches", "value"),
               State("date_range", "start_date"),
               State("date_range", "end_date")])
def update_float_metrics(_, sensor_name, control_switches, sd, ed):
    df, sensor_doc, min_max, with_setpoint = get_and_merge_data(sensor_name["sensor"], sd, ed)

    figure = go.Figure().add_trace(go.Scatter(
                    x = df.index,
                    y = df["val"],
                    mode = "lines+markers",
                    name = "Read"
                    )
                )

    set_style(figure, sensor_doc["display"], control_switches, min_max)
    return figure


@app.callback(Output("control_switches", "value"),
        [Input({"type": "float_sensor", "sensor": ALL}, 'relayoutData')],
        [State("control_switches", "value")]
        )
def update_output_div(zoom, switches):
    zoom_changes = ["xaxis.range" in _ for _ in zoom if _ is not None]
    if sum(zoom_changes) == len(zoom) or sum(zoom_changes) == 0:
        return dash.no_update
    if 1 in switches:
        switches.remove(1)
    return switches


def set_style(figure, title, control_switches, data_range):
    offset_minutes = 2
    data_range[0] -= datetime.timedelta(minutes = 20)
    data_range[1] += datetime.timedelta(minutes = 20)
    xaxis = dict()
    if 1 in control_switches:
        now = from_utc()
        init_range = [now - datetime.timedelta(hours = 5), now + datetime.timedelta(minutes = offset_minutes)]
        xaxis.update({"rangeslider_visible": True,
              "range": init_range,
              "rangeslider_range": data_range, })

    xaxis["rangeselector"] = dict(
        buttons = list([
            dict(count = 1, label = "1h", step = "hour", stepmode = "todate"),
            dict(count = 30 + offset_minutes, label = "30m", step = "minute", stepmode = "todate"),
            dict(count = 5 + offset_minutes, label = "5m", step = "minute", stepmode = "todate"),
            dict(step = "all")
        ],),
        font_color = 'black'
    )

    figure.update_layout(
    title = dict(text = title,
             x = 0.5,),
    margin_t = 50,
    margin_b = 10,
    height = 200,
    xaxis = xaxis,
    # uirevision = "keep",
    template = "plotly_dark",
    )
