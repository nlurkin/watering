#!/bin/env python
'''
Created on 87 Jun 2020

@author: Nicolas Lurkin
'''

import dash
import dash_html_components as html
import dash_bootstrap_components as dbc
import dash_core_components as dcc
from dash.dependencies import Input, Output
from waterapp import app
from apps import dashboard, add_sensor, add_dashboard
from waterapp import mongoClient

left_col = []
right_col = []

# Define the app
index_layout = html.Div(
        children = [
        dcc.Location(id = 'url', refresh = False),
        dbc.Row(html.H2("Watering dashboard", className = "title"), className = "black"),
        dcc.Link("Home", href = "/"),
        html.Div(id = "page-content")
        ]
    )


def get_dashboard_list():
    elements = [
        html.H3("Add new elements"),
        dbc.Row([dbc.Col(dcc.Link('New dashboard', href = '/add/dashboard'), width=1),
                 dbc.Col(dcc.Link('Update dashboard', href = '/update/dashboard'), width=1)]),
        dbc.Row([dbc.Col(dcc.Link('New sensor', href = '/add/sensor'), width=1),
                 dbc.Col(dcc.Link('Update sensor', href = '/update/sensor'), width=1)]),
        dbc.Row(dbc.Col(dcc.Link('New controller', href = '/add/controller'), width=1)),
        html.H3("Available dashboards"), ]

    db_list = mongoClient.get_dashboards_dropdown()
    for db in db_list:
        elements.append(
            dbc.Row(dbc.Col(dcc.Link(db["label"], href = f"/dashboard/{db['label']}"))))

    return elements


app.layout = index_layout


@app.callback(Output('page-content', 'children'),
              [Input('url', 'pathname')])
def display_page(pathname):
    if pathname is None:
        return dash.no_update
    if pathname == "/":
        return get_dashboard_list()
    elif pathname == "/add/sensor":
        return add_sensor.get_layout(update=False)
    elif pathname == "/update/sensor":
        return add_sensor.get_layout(update=True)
    elif pathname == "/add/dashboard":
        return add_dashboard.layout
    elif pathname[:11] == '/dashboard/':
        return dashboard.generate_layout(pathname.split("/")[-1])
    else:
        return '404'


# Run the app
if __name__ == '__main__':
    app.run_server(debug = True)
