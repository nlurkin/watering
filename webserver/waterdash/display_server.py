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
from dash.dependencies import Input, Output, ALL
from waterapp import app
from apps import dashboard, add_sensor, add_dashboard, publications, config_menu, main_page
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
        dbc.Nav(
            [
                dbc.NavItem(dbc.NavLink("Home", id = {"type":"nav", "id":""}, href = app_prefix + "/")),
                dbc.NavItem(dbc.NavLink("Configuration", id = {"type":"nav", "id":"config"}, href = app_prefix + "/config")),
            ],
            pills = True
            ),
        html.Div(id = "page-content")
        ]
    )


app.layout = index_layout


@app.callback([Output({"type": "nav", 'id': ALL}, 'active')],
              [Input('url', 'pathname')],
              [])
def navbar_state(pathname):
    active_link = ([pathname == f'/{i}' for i in ["", "config"]])
    return (active_link,)


@app.callback(Output('page-content', 'children'),
              [Input('url', 'pathname')])
def display_page(pathname):
    if pathname is None:
        return dash.no_update
    if pathname == app_prefix + "/":
        return main_page.get_layout()
    if pathname == app_prefix + "/config":
        return config_menu.get_layout()
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
