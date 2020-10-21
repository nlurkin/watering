#!/bin/env python
'''
Created on 87 Jun 2020

@author: Nicolas Lurkin
'''

import dash
import dash_html_components as html
import dash_bootstrap_components as dbc
import dash_core_components as dcc
from dash.dependencies import Input, Output, State, ALL, MATCH
import plotly.graph_objects as go
import pandas as pd
from waterapp import app, mongoClient
from datetime import datetime


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
            day = datetime.now().strftime("%Y-%m-%d")
            controller_last_value = mongoClient.get_controller_values(sensor["sensor"], day)
            sensor_element.append(dbc.Checklist(options = [{"label": "", "value": 1}],
            value = [] if (len(controller_last_value) == 0 or controller_last_value[-1]["val"] == 0) else [1], id = {"type": "bool_controller", "sensor": sensor["sensor"]}, switch = True,))
            sensor_element.append(html.P(id = {"type": "dummy", "sensor": sensor["sensor"]}))

    return dbc.Card(sensor_element)


def generate_dashboard_column(col_description):
    col = []
    sensor_docs = mongoClient.get_sensor_by_id(col_description)
    for sensor in col_description:
        col.append(build_sensor_card(sensor_docs[sensor]))
    return col


def generate_layout(dashboard_name):
    db_doc = mongoClient.get_dashboard_by_name(dashboard_name)
    left_col = generate_dashboard_column(db_doc["left"])
    right_col = generate_dashboard_column(db_doc["right"])

    return [dcc.Interval(id = 'interval-component', interval = 1 * 1000, n_intervals = 0),
            dbc.Row([dbc.Col(left_col, style = {"padding": "0px"}), dbc.Col(right_col, style = {"padding": "0px"})])
            ]


@app.callback(Output({"type": "string_sensor", "sensor": MATCH}, 'value'),
              [Input('interval-component', 'n_intervals')],
              [State({"type": "string_sensor", "sensor": MATCH}, 'id')])
def update_string_metrics(_, sensor_name):
    day = datetime.now().strftime("%Y-%m-%d")
    value_doc = mongoClient.get_sensor_values(sensor_name["sensor"], day)
    return "".join([_["val"] for _ in value_doc["samples"]])


@app.callback(Output({"type": "bool_sensor", "sensor": MATCH}, 'figure'),
              [Input('interval-component', 'n_intervals')],
              [State({"type": "bool_sensor", "sensor": MATCH}, 'id')])
def update_bool_metrics(_, sensor_name):
    day = datetime.now().strftime("%Y-%m-%d")
    value_doc = mongoClient.get_sensor_values(sensor_name["sensor"], day)
    sensor_doc = mongoClient.get_sensor_by_name(sensor_name["sensor"])
    with_setpoint = False
    if value_doc is None:
        df = pd.DataFrame({"val": []})
    else:
        if "samples" not in value_doc:
            df = pd.DataFrame({"val": []})
        else:
            df = pd.DataFrame(value_doc["samples"])
            df.index = pd.to_datetime(df['ts'], unit = "s")
        if "setpoint" in value_doc:
            dsp = pd.DataFrame(value_doc["setpoint"])
            dsp.index = pd.to_datetime(dsp['ts'], unit = "s")
            dsp = dsp.rename(columns = {"val": "sp"})
            df = pd.merge(df, dsp, how = 'outer', left_index = True, right_index = True)
            with_setpoint = True

    figure = go.Figure(
        layout_title_text = sensor_doc["display"],
        layout_title_x = 0.5,
        layout_margin_t = 50,
        layout_margin_b = 10,
        layout_height = 200,
        layout_template = "plotly_dark",
        ).add_trace(go.Scatter(
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
                    name = "Set"
                    )
        )
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

    ts = datetime.now().timestamp()
    day = datetime.now().strftime("%Y-%m-%d")
    val = 0 if len(controller_value) == 0 else 1
    mongoClient.update_controller_values(sensor_doc, sensor_name, val, day, ts)

    return ""
