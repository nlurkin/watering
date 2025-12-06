#!/bin/env python
"""
Created on 87 Jun 2020

@author: Nicolas Lurkin
"""

import dash_bootstrap_components as dbc
from dash import dcc, html

from data.config import app_prefix
from waterapp import mongoClient


def get_layout():
    elements = [
        html.H3("Add new elements"),
        dbc.Row(
            [
                dbc.Col(
                    dcc.Link("New dashboard", href=app_prefix + "/add/dashboard"),
                    width=1,
                ),
                dbc.Col(
                    dcc.Link("Update dashboard", href=app_prefix + "/update/dashboard"),
                    width=1,
                ),
            ]
        ),
        dbc.Row(
            [
                dbc.Col(
                    dcc.Link("New sensor", href=app_prefix + "/add/sensor"), width=1
                ),
                dbc.Col(
                    dcc.Link("Update sensor", href=app_prefix + "/update/sensor"),
                    width=1,
                ),
                dbc.Col(
                    dcc.Link("List sensors", href=app_prefix + "/list/sensor"), width=1
                ),
                dbc.Col(
                    dcc.Link(
                        "List publications", href=app_prefix + "/list/publications"
                    ),
                    width=1,
                ),
            ]
        ),
        html.H3("Available dashboards"),
    ]

    db_list = mongoClient.get_dashboards_dropdown()
    for db in db_list:
        elements.append(
            dbc.Row(
                dbc.Col(
                    dcc.Link(db["label"], href=f"{app_prefix}/dashboard/{db['label']}")
                )
            )
        )

    return elements
