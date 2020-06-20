#!/bin/env python
'''
Created on 87 Jun 2020

@author: Nicolas Lurkin
'''

import dash_html_components as html
import dash_bootstrap_components as dbc
import dash_core_components as dcc
from dash.dependencies import Input, Output, State, ALL, MATCH
import plotly.graph_objects as go
import pandas as pd
from waterapp import app, server, mongoClient
from bson.objectid import ObjectId
from datetime import datetime

df = pd.read_csv('data/stockdata2.csv', parse_dates = True)
df.index = pd.to_datetime(df['Date'])

# graph = dcc.Graph(
#    id = 'pump1_status',
#    # config = {'displayModeBar': False},
#    animate = True,
#    figure = go.Figure(
#        layout_title_text = "Pump1 status",
#        layout_title_x = 0.5,
#        layout_margin_t = 50,
#        layout_margin_b = 10,
#        layout_height = 300,
#        layout_template = "plotly_dark",
#        )
#        .add_trace(go.Scatter(
#            x = df['Date'],
#            y = df['value'],
#            mode = "lines+markers",
#            ))
#    )
#
# graph2 = html.H2("Text")


def build_sensor_card(sensor):
    sensor_element = None
    if sensor["data-type"] == "string":
        sensor_element = dcc.Textarea(
          value = '',
          style = {'width': '100%', 'height': '300px'},
          id = {"type": "string_sensor", "sensor": sensor["sensor"]},
          readOnly = True,
          contentEditable = True,
          persistence = False,
          rows = 30,
        )

    return dbc.Card(sensor_element)


def generate_dashboard_column(col_description):
    col = []
    sensor_docs = mongoClient.sensors_db.find({"_id": {"$in": [ObjectId(_) for _ in col_description]}})
    for sensor in sensor_docs:
        col.append(build_sensor_card(sensor))
    return col


def generate_layout(dashboard_name):
    db_doc = mongoClient.dashboard_db.find_one({"name": dashboard_name})
    left_col = generate_dashboard_column(db_doc["left"])
    right_col = generate_dashboard_column(db_doc["right"])

    return [dcc.Interval(id = 'interval-component', interval = 1 * 1000, n_intervals = 0),
            dbc.Row([dbc.Col(left_col, style = {"padding": "0px"}), dbc.Col(right_col, style = {"padding": "0px"})])
            ]


@app.callback(Output({"type": "string_sensor", "sensor": MATCH}, 'value'),
              [Input('interval-component', 'n_intervals')],
              [State({"type": "string_sensor", "sensor": MATCH}, 'id')])
def update_metrics(_, sensor_name):
    day = datetime.now().strftime("%Y-%m-%d")
    value_doc = mongoClient.client["sensors"][sensor_name["sensor"]].find_one({"day": day})
    return "\n".join([_["val"] for _ in value_doc["samples"]])
