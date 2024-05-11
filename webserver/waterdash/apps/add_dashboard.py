"""
Created on 18-Jun-2020

@author: Nicolas Lurkin
"""

import re

import dash
import dash_bootstrap_components as dbc
from dash import dcc, html
from dash.dependencies import ALL, Input, Output, State

from waterapp import app, mongoClient


def get_layout(update):
    if update:
        field_to_use = [
            dbc.Input(
                id="add_dashboard_name",
                placeholder="Dashboard name",
                type="text",
                pattern="[a-zA-Z0-9_]+",
                style={"display": "None"},
            ),
            dcc.Dropdown(
                id="update_dashboard_name",
                placeholder="Dashboard name",
                options=mongoClient.get_dashboards_dropdown(),
            ),
        ]
    else:
        field_to_use = [
            dbc.Input(
                id="add_dashboard_name",
                placeholder="Dashboard name",
                type="text",
                pattern="[a-zA-Z0-9_]+",
            ),
            dcc.Dropdown(
                id="update_dashboard_name",
                placeholder="Dashboard name",
                style={"display": "None"},
            ),
        ]

    return [
        html.Div(
            [
                dbc.Row(
                    [
                        dbc.Label(
                            "Dashboard name", html_for="add_dashboard_name", width=2
                        ),
                        dbc.Col(field_to_use, width=2),
                        dbc.Col(
                            dbc.Button(
                                "Save dashboard",
                                id="add_dashboard_submit",
                                color="primary",
                            ),
                            width=2,
                        ),
                        dbc.Tooltip(
                            "Only alphanumerical characters and _ allowed",
                            target="add_dashboard_name",
                            placement="right",
                            className="mytooltip",
                        ),
                    ],
                ),
                dbc.Row(
                    dbc.Row(
                        children=[
                            dbc.Col(
                                [html.Div()],
                                id={"type": "group_cards", "index": "left"},
                            ),
                            dbc.Col(
                                [html.Div()],
                                id={"type": "group_cards", "index": "right"},
                            ),
                        ]
                    )
                ),
                dbc.Row(
                    dbc.Row(
                        [
                            dbc.Col(
                                dbc.Card(
                                    dbc.Button(
                                        "Add Element",
                                        id={"type": "add_element", "index": "left"},
                                    )
                                )
                            ),
                            dbc.Col(
                                dbc.Card(
                                    dbc.Button(
                                        "Add Element",
                                        id={"type": "add_element", "index": "right"},
                                    )
                                )
                            ),
                        ]
                    ),
                    id="group_add",
                ),
            ],
            className="form",
        ),
        dbc.Modal(
            [
                dbc.ModalBody("", id="add_dashboard_modal_message"),
                dbc.ModalFooter(
                    dbc.Button(
                        "Close", id="add_dashboard_modal_close", className="ml-auto"
                    )
                ),
            ],
            id="add_dashboard_modal",
            size="sm",
        ),
        dbc.Modal(
            [
                dbc.ModalBody("", id="update_dashboard_modal_message"),
                dbc.ModalFooter(
                    dbc.Button(
                        "Close", id="update_dashboard_modal_close", className="ml-auto"
                    )
                ),
            ],
            id="update_dashboard_modal",
            size="sm",
        ),
    ]


# ========================
# Callbacks
# ========================
@app.callback(
    [
        Output("update_dashboard_modal", "is_open"),
        Output("update_dashboard_modal_message", "className"),
        Output("update_dashboard_modal_message", "children"),
        Output({"type": "group_cards", "index": "left"}, "children"),
        Output({"type": "group_cards", "index": "right"}, "children"),
    ],
    [
        Input({"type": "add_element", "index": "left"}, "n_clicks"),
        Input({"type": "add_element", "index": "right"}, "n_clicks"),
        Input("update_dashboard_name", "value"),
    ],
    [
        State({"type": "group_cards", "index": "left"}, "children"),
        State({"type": "group_cards", "index": "right"}, "children"),
    ],
)
def update_layout(btn_add_left, btn_add_right, update_name, left_col, right_col):
    ctx = dash.callback_context
    if not ctx.triggered:
        return dash.no_update

    trig_id = ctx.triggered[0]["prop_id"].split(".")[0]
    if "add_element" in trig_id:
        trig_id = eval(trig_id)
        if trig_id["index"] == "left":
            return (
                False,
                "",
                "",
                add_element_col(None, left_col, "left"),
                dash.no_update,
            )
        if trig_id["index"] == "right":
            return (
                False,
                "",
                "",
                dash.no_update,
                add_element_col(None, right_col, "right"),
            )
    elif trig_id == "update_dashboard_name":
        return build_layout(update_name)


@app.callback(
    [
        Output("add_dashboard_modal", "is_open"),
        Output("add_dashboard_modal_message", "className"),
        Output("add_dashboard_modal_message", "children"),
    ],
    [
        Input("add_dashboard_submit", "n_clicks"),
        Input("add_dashboard_modal_close", "n_clicks"),
    ],
    [
        State("add_dashboard_name", "value"),
        State("add_dashboard_name", "pattern"),
        State("update_dashboard_name", "value"),
        State({"type": "sensor_list", "index": ALL}, "value"),
        State({"type": "sensor_list", "index": ALL}, "id"),
        State("add_dashboard_modal", "is_open"),
    ],
)
def save_dashboard(
    submit,
    close,
    dashboard_name,
    dashboard_name_pattern,
    update_id,
    sensors,
    sensors_ids,
    is_open,
):
    ctx = dash.callback_context
    if not ctx.triggered or is_open:
        return False, "", ""

    print(dashboard_name, update_id)
    left_sensors, right_sensors = build_sensor_lists(sensors, sensors_ids)
    if dashboard_name is None and update_id is not None:
        return update_dashboard(update_id, left_sensors, right_sensors)
    elif dashboard_name is not None and update_id is None:
        return add_dashboard(
            dashboard_name, dashboard_name_pattern, left_sensors, right_sensors
        )

    return dash.no_update


# =======================
# Update functions
# =======================
def generate_dashboard_column(col_description, index):
    col = [html.Div()]
    sensor_docs = mongoClient.get_sensor_by_id(col_description)
    for sensor in col_description:
        if not sensor in sensor_docs:
            add_element_col(None, col, index, None)
        else:
            add_element_col(None, col, index, str(sensor_docs[sensor]["_id"]))
    return col


def build_layout(dashboard_name):
    dashboard_doc = mongoClient.get_dashboard_by_id(dashboard_name)
    if dashboard_doc is None:
        return (
            True,
            "message-error",
            f"Unable to find sensor id {dashboard_name}",
            dash.no_update,
            dash.no_update,
        )

    left_col = generate_dashboard_column(dashboard_doc["left"], "left")
    right_col = generate_dashboard_column(dashboard_doc["right"], "right")

    return False, "", "", left_col, right_col


def add_element_col(_, column, index, value=None):
    elt_num = len(column) - 1
    db_sensor_list = mongoClient.get_sensors_dropdown()
    sensor_list = dcc.Dropdown(
        id={"type": "sensor_list", "index": f"{index}_{elt_num}"},
        options=db_sensor_list,
        placeholder="Select a sensor or controller",
        value=value,
    )
    column.append(
        dbc.Card(
            [
                html.Div(id={"type": "sensor_content", "index": f"{index}_{elt_num}"}),
                sensor_list,
            ]
        )
    )
    return column


def build_sensor_lists(sensors, sensors_ids):
    left_sensors = {}
    right_sensors = {}
    for sensor, sensor_id in zip(sensors, sensors_ids):
        col, my_id = sensor_id["index"].split("_")
        if col == "left":
            left_sensors[int(my_id)] = sensor
        if col == "right":
            right_sensors[int(my_id)] = sensor

    left_sensors = [left_sensors[_] for _ in sorted(left_sensors)]
    right_sensors = [right_sensors[_] for _ in sorted(right_sensors)]

    return left_sensors, right_sensors


def update_dashboard(dashboard_id, left_sensors, right_sensors):
    db_doc = mongoClient.get_dashboard_by_id(dashboard_id)
    if db_doc is None:
        return True, "message-error", f"A dashboard with ID {dashboard_id} not found."

    data_dict = {"left": left_sensors, "right": right_sensors}

    mongoClient.update_dashboard_by_id(dashboard_id, data_dict)
    return (
        True,
        "message-valid",
        f"Dashboard with ID:{dashboard_id} successfully updated",
    )


def add_dashboard(dashboard_name, dashboard_name_pattern, left_sensors, right_sensors):
    if dashboard_name is None or not re.match(dashboard_name_pattern, dashboard_name):
        return (
            True,
            "message-error",
            "Dashboard name is invalid, only alphanumerical characters and _ are allowed",
        )

    db_doc = mongoClient.get_dashboard_by_name(dashboard_name)
    if db_doc is not None:
        return (
            True,
            "message-error",
            f"A dashboard with same name already exists. Choose another name.",
        )

    data_dict = {"name": dashboard_name, "left": left_sensors, "right": right_sensors}

    db_doc = mongoClient.add_dashboard(data_dict)
    return (
        True,
        "message-valid",
        f"Dashboard successfully added with ID:{db_doc.inserted_id}",
    )
