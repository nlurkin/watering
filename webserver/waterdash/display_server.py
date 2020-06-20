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


app.layout = index_layout


@app.callback(Output('page-content', 'children'),
              [Input('url', 'pathname')])
def display_page(pathname):
    if pathname is None:
        return dash.no_update
    else:
        return '404'


# Run the app
if __name__ == '__main__':
    app.run_server(debug = True)
