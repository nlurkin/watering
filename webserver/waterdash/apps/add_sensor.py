'''
Created on 18-Jun-2020

@author: Nicolas Lurkin
'''

import dash
import dash_core_components as dcc
import dash_html_components as html
import dash_bootstrap_components as dbc
from dash.dependencies import Input, Output, State
from waterapp import app, mongoClient
import re

layout = [html.Div(
    [
        dbc.FormGroup([
            dbc.Label("Sensor name", html_for = "add_sensor_name", width = 2),
            dbc.Col(dcc.Dropdown(id = "add_sensor_name", placeholder = "Sensor name", type = "text", pattern = "[a-zA-Z0-9_]+"), width = 2),
            ], row = True),
        dbc.FormGroup([
            dbc.Label("Sensor display name", html_for = "add_sensor_display", width = 2),
            dbc.Col(dbc.Input(id = "add_sensor_display", placeholder = "Sensor display name", type = "text"), width = 2),
            ], row = True),
        dbc.FormGroup([
            dbc.Label("Sensor data type", html_for = "add_sensor_type", width = 2),
            dbc.Col(dcc.Dropdown(id = 'add_sensor_type',
                                 options = [
                                     {'label': 'Numerical', 'value': 'float'},
                                     {'label': 'Boolean', 'value': 'bool'},
                                     {'label': 'Text', 'value': 'string'}
                                     ],
                                 placeholder = "Select a data type",
                                 searchable = False,
                                 clearable = False), width = 2),
            ], row = True),
        dbc.FormGroup([
            dbc.Col(width = 2),
            dbc.Col(dbc.Button("Submit", id = "add_sensor_submit", color = "primary"), width = 2),
            ], row = True),
        ],
    className = "form"),
    dbc.Modal([
        dbc.ModalBody("", id = "add_sensor_modal_message"),
        dbc.ModalFooter(
            dbc.Button("Close", id = "add_sensor_modal_close", className = "ml-auto")
            ),
        ],
        id = "add_sensor_modal",
        size = "sm"
        ),
    ]

@app.callback([Output('add_sensor_modal', 'is_open'), Output('add_sensor_modal_message', 'className'), Output('add_sensor_modal_message', 'children')],
              [Input('add_sensor_submit', 'n_clicks'), Input("add_sensor_modal_close", "n_clicks")],
              [State('add_sensor_name', 'value'),
               State('add_sensor_name', 'pattern'),
               State('add_sensor_display', 'value'),
               State('add_sensor_type', 'value'),
               State("add_sensor_modal", "is_open")])
def update_output(submit, close, name, name_pattern, display, stype, is_open):
    ctx = dash.callback_context
    if not ctx.triggered or is_open:
        return False, "", ""

    if name is None or stype is None or not re.match(name_pattern, name):
        return True, "message-error", "Sensor name is invalid, only alphanumerical characters and _ are allowed"
    if display is None:
        display = name

        data_dict = {"sensor": name, "display": display, "data-type": stype}
        sensor_doc = mongoClient.get_sensor_by_name(name)
        if sensor_doc is not None:
            return True, "message-error", f"A sensor with same name already exists. Choose another name."

    sensor_id = mongoClient.add_sensor(data_dict)
    return True, "message-valid", f"Sensor successfully added with ID:{sensor_id}"

