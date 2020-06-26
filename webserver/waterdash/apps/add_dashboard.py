'''
Created on 18-Jun-2020

@author: Nicolas Lurkin
'''

import dash
import dash_core_components as dcc
import dash_html_components as html
import dash_bootstrap_components as dbc
from dash.dependencies import Input, Output, State, ALL, MATCH
from waterapp import app, mongoClient
import re

layout = [html.Div(
        [
            dbc.FormGroup([
                dbc.Label("Dashboard name", html_for = "add_dashboard_name", width = 2),
                dbc.Col(dbc.Input(id = "add_dashboard_name", placeholder = "Dashboard name", type = "text", pattern = "[a-zA-Z0-9_]+"), width = 2),
                dbc.Col(dbc.Button("Save dashboard", id = "add_dashboard_submit", color = "primary"), width = 2),
                dbc.Tooltip("Only alphanumerical characters and _ allowed", target = "add_dashboard_name", placement = "right", className = "mytooltip"),
                ], row = True),
            dbc.FormGroup(dbc.Row(children = [dbc.Col([html.Div()], id = {"type": "group_cards", "index":"left"}), dbc.Col([html.Div()], id = {"type": "group_cards", "index":"right"})])),
            dbc.FormGroup(dbc.Row([
                dbc.Col(dbc.Card(dbc.Button("Add Element", id = {"type": "add_element", "index": "left"}))),
                dbc.Col(dbc.Card(dbc.Button("Add Element", id = {"type": "add_element", "index": "right"})))
                ]), id = "group_add"),
            ],
            className = "form"),
            dbc.Modal([
                dbc.ModalBody("", id = "add_dashboard_modal_message"),
                dbc.ModalFooter(
                    dbc.Button("Close", id = "add_dashboard_modal_close", className = "ml-auto")
                ),
            ],
            id = "add_dashboard_modal",
            size = "sm"
        ),
    ]


@app.callback(Output({"type": "group_cards", "index": MATCH}, "children"),
              [Input({"type": "add_element", "index": MATCH}, 'n_clicks')],
              [State({"type": "group_cards", "index": MATCH}, "children"),
               State({"type": "group_cards", "index": MATCH}, "id"), ])
def add_element_left(_, column, group_id):
    ctx = dash.callback_context
    if not ctx.triggered:
        return column
    elt_num = len(column) - 1
    db_sensor_list = mongoClient.get_sensors_dropdown()
    sensor_list = dcc.Dropdown(
        id = {"type": "sensor_list", "index":f"{group_id['index']}_{elt_num}"},
        options = db_sensor_list,
        placeholder = "Select a sensor"
        )
    print(column, group_id)
    column.append(dbc.Card([html.Div(id = {"type": "sensor_content", "index": f"{group_id['index']}_{elt_num}"}), sensor_list]))
    print(column)
    return column


@app.callback(Output({"type": "sensor_content", "index": MATCH}, "children"),
              [Input({"type": 'sensor_list', "index": MATCH}, "value"),
               Input({"type": 'sensor_list', "index": MATCH}, "id")])
def update_card_sensor(sensor_name, sensor_id):
    ctx = dash.callback_context
    if not ctx.triggered:
        return dash.no_update
    return f"Displaying sensor {sensor_name} {sensor_id}"


@app.callback([Output('add_dashboard_modal', 'is_open'), Output('add_dashboard_modal_message', 'className'), Output('add_dashboard_modal_message', 'children')],
              [Input('add_dashboard_submit', 'n_clicks'), Input("add_dashboard_modal_close", "n_clicks")],
              [State("add_dashboard_name", "value"),
               State("add_dashboard_name", "pattern"),
               State({"type": "sensor_list", "index": ALL}, 'value'),
               State({"type": "sensor_list", "index": ALL}, 'id'),
               State("add_dashboard_modal", "is_open")])
def update_output(submit, close, dashboard_name, dashboard_name_pattern, sensors, sensors_ids, is_open):
    ctx = dash.callback_context
    if not ctx.triggered or is_open:
        return False, "", ""

    if dashboard_name is None or not re.match(dashboard_name_pattern, dashboard_name):
        return True, "message-error", "Dashboard name is invalid, only alphanumerical characters and _ are allowed"

    db_doc = mongoClient.get_dashboard_by_name(dashboard_name)
    if db_doc is not None:
        return True, "message-error", f"A dashboard with same name already exists. Choose another name."

    print(submit, close, sensors, sensors_ids)

    left_sensors = {}
    right_sensors = {}
    for sensor, sensor_id in zip(sensors, sensors_ids):
        print(sensor_id)
        col, my_id = sensor_id["index"].split("_")
        if col == "left":
            left_sensors[int(my_id)] = sensor
        if col == "right":
            right_sensors[int(my_id)] = sensor

    left_sensors = [left_sensors[_] for _ in sorted(left_sensors)]
    right_sensors = [right_sensors[_] for _ in sorted(right_sensors)]
    data_dict = {"name": dashboard_name, "left": left_sensors, "right": right_sensors}

    db_doc = mongoClient.add_dashboard(data_dict)
    return True, "message-valid", f"Dashboard successfully added with ID:{db_doc.inserted_id}"

