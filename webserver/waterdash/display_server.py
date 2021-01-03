#!/bin/env python
'''
Created on 87 Jun 2020

@author: Nicolas Lurkin
'''

import dash
import dash_auth
import dash_html_components as html
import dash_bootstrap_components as dbc
import dash_core_components as dcc
from dash.dependencies import Input, Output
from waterapp import app, server
from apps import dashboard, add_sensor, add_dashboard, publications
from waterapp import mongoClient
from data.config import VALID_USERNAME_PASSWORD_PAIRS, app_prefix

auth = dash_auth.BasicAuth(
    app,
    VALID_USERNAME_PASSWORD_PAIRS
)

left_col = []
right_col = []

# Define the app
index_layout = html.Div(
        children = [
        dcc.Location(id = 'url', refresh = False),
        dbc.Row(html.H2("Watering dashboard", className = "title"), className = "black"),
        dcc.Link("Home", href = app_prefix + "/"),
        html.Div(id = "page-content")
        ]
    )


def get_dashboard_list():
    elements = [
        html.H3("Add new elements"),
        dbc.Row([dbc.Col(dcc.Link('New dashboard', href = app_prefix + '/add/dashboard'), width = 1),
                 dbc.Col(dcc.Link('Update dashboard', href = app_prefix + '/update/dashboard'), width = 1)]),
        dbc.Row([dbc.Col(dcc.Link('New sensor', href = app_prefix + '/add/sensor'), width = 1),
                 dbc.Col(dcc.Link('Update sensor', href = app_prefix + '/update/sensor'), width = 1),
                 dbc.Col(dcc.Link('List sensors', href = app_prefix + '/list/sensor'), width = 1),
                 dbc.Col(dcc.Link('List publications', href = app_prefix + '/list/publications'), width = 1)]),
        html.H3("Available dashboards"), ]

    db_list = mongoClient.get_dashboards_dropdown()
    for db in db_list:
        elements.append(
            dbc.Row(dbc.Col(dcc.Link(db["label"], href = f"{app_prefix}/dashboard/{db['label']}"))))

    return elements


app.layout = index_layout


@app.callback(Output('page-content', 'children'),
              [Input('url', 'pathname')])
def display_page(pathname):
    if pathname is None:
        return dash.no_update
    if pathname == app_prefix + "/":
        return get_dashboard_list()
    elif pathname == app_prefix + "/add/sensor":
        return add_sensor.get_layout(update = False)
    elif pathname == app_prefix + "/update/sensor":
        return add_sensor.get_layout(update = True)
    elif pathname == app_prefix + "/list/sensor":
        return add_sensor.get_layout(dolist = True)
    elif pathname == app_prefix + "/list/publications":
        return publications.get_layout()
    elif pathname == app_prefix + "/add/dashboard":
        return add_dashboard.get_layout(update = False)
    elif pathname == app_prefix + "/update/dashboard":
        return add_dashboard.get_layout(update = True)
    elif pathname[:11 + len(app_prefix)] == app_prefix + '/dashboard/':
        return dashboard.generate_layout(pathname.split("/")[-1])
    else:
        return '404'


# Run the app
if __name__ == '__main__':
    app.run_server(debug = True)
