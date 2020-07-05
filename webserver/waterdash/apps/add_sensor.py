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


def get_layout(update = False, dolist = False):
    if dolist:
        return build_list_layout()
    else:
        return build_mod_layout(update)


def build_list_layout():
    t_header = [html.Thead(html.Tr([html.Th("Sensor"), html.Th("Display Name"), html.Th("Type"), html.Th("Controller?")]))]

    t_body = [html.Tr([
        html.Td(sensor['sensor']), html.Td(sensor['display']),
        html.Td(sensor['data-type']), html.Td(dbc.Checklist(options = [{"label": "", "value": 1, "disabled":True}],
            value = [1] if sensor["controller"] else [], id = sensor["sensor"], switch = True,))
        ]) for sensor in mongoClient.get_sensors_list()]

    return [html.Div(dbc.Table(t_header + t_body, dark = True, striped = True))]


def build_mod_layout(update):
    if update:
        field_to_use = [dbc.Input(id = "add_sensor_name", placeholder = "Sensor name", type = "text", pattern = "[a-zA-Z0-9_]+", style = {"display": "None"}),
                        dcc.Dropdown(id = "update_sensor_name", placeholder = "Sensor name", options = mongoClient.get_sensors_dropdown())]
        button_text = "Update"
    else:
        field_to_use = [dbc.Input(id = "add_sensor_name", placeholder = "Sensor name", type = "text", pattern = "[a-zA-Z0-9_]+"),
                        dcc.Dropdown(id = "update_sensor_name", placeholder = "Sensor name", options = mongoClient.get_sensors_dropdown(), style = {"display": "None"})]
        button_text = "Add"

    return [html.Div(
    [
        dbc.FormGroup([
            dbc.Label("Sensor name", html_for = "add_sensor_name", width = 2),
            dbc.Col(field_to_use, width = 2),
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
            dbc.Label("", html_for = "add_sensor_controller", width = 2),
            dbc.Col(dbc.Checklist(id = "add_sensor_controller",
                                  options = [{"label": "Is controller", "value": 1}, ],
                                  switch = True), width = 2),
            ], row = True),
        dbc.FormGroup([
            dbc.Col(width = 2),
            dbc.Col(dbc.Button(button_text, id = "add_sensor_submit", color = "primary"), width = 2),
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
    dbc.Modal([
        dbc.ModalBody("", id = "update_sensor_modal_message"),
        dbc.ModalFooter(
            dbc.Button("Close", id = "update_sensor_modal_close", className = "ml-auto")
            ),
        ],
        id = "update_sensor_modal",
        size = "sm"
        ),
    ]


@app.callback([Output('update_sensor_modal', 'is_open'), Output('update_sensor_modal_message', 'className'), Output('update_sensor_modal_message', 'children'),
               Output("add_sensor_display", "value"), Output("add_sensor_type", "value")],
               [Input("update_sensor_name", "value")])
def update_display(sensor_name):
    ctx = dash.callback_context
    if not ctx.triggered:
        return dash.no_update

    sensor_doc = mongoClient.get_sensor_by_id(sensor_name)
    if sensor_doc is None:
        return True, "message-error", f"Unable to find sensor id {sensor_name}", dash.no_update, dash.no_update

    return False, "", "", sensor_doc["display"], sensor_doc["data-type"]


@app.callback([Output('add_sensor_modal', 'is_open'), Output('add_sensor_modal_message', 'className'), Output('add_sensor_modal_message', 'children')],
              [Input('add_sensor_submit', 'n_clicks'), Input("add_sensor_modal_close", "n_clicks")],
              [State('add_sensor_name', 'value'),
               State('add_sensor_name', 'pattern'),
               State('update_sensor_name', 'value'),
               State('add_sensor_display', 'value'),
               State('add_sensor_type', 'value'),
               State('add_sensor_controller', 'value'),
               State("add_sensor_modal", "is_open")])
def create_update_sensor(submit, close, name, name_pattern, update_name, display, stype, is_controller, is_open):
    ctx = dash.callback_context
    if not ctx.triggered or is_open:
        return False, "", ""

    if name is not None:
        # We are adding a new sensor
        if name is None or stype is None or not re.match(name_pattern, name):
            return True, "message-error", "Sensor name is invalid, only alphanumerical characters and _ are allowed"
        if display is None:
            display = name

        data_dict = {"sensor": name, "display": display, "data-type": stype, "controller": is_controller == 1}
        sensor_doc = mongoClient.get_sensor_by_name(name)
        if sensor_doc is not None:
            return True, "message-error", f"A sensor with same name already exists. Choose another name."

        sensor_id = mongoClient.add_sensor(data_dict)
        return True, "message-valid", f"Sensor successfully added with ID:{sensor_id}"

    elif update_name is not None:
        # We are updating a sensor
        data_dict = {"display": display, "data-type": stype, "controller": is_controller == 1}
        mongoClient.update_sensor_by_id(update_name, data_dict)
        return True, "message-valid", f"Sensor with ID {update_name} successfully updated"

