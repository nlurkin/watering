'''
Created on 21 Mar 2021

@author: Nicolas Lurkin
'''

from dash.dependencies import MATCH, Output, Input, State

import dash_bootstrap_components as dbc
import dash_core_components as dcc
import dash_html_components as html
from mongodb import from_utc, to_utc
import pandas as pd
import plotly.graph_objects as go
from waterapp import mongoClient, app
import datetime
import dateutil


def make_location_summary():
    latest = mongoClient.get_latest_sensor_value("bme1_temperature")
    indoor = dbc.Row([html.Img(src = "assets/in_house.png", height = "30px"), html.Div(latest, id = {"type":"local_value", "sensor":"bme1_temperature"}), "\u00B0C"], className = "weather_row")
    latest = mongoClient.get_latest_sensor_value("428F_94_temp")
    outdoor = dbc.Row([html.Img(src = "assets/out_house.png", height = "30px"), html.Div(latest, id = {"type":"local_value", "sensor":"428F_94_temp"}), "\u00B0C"], className = "weather_row", style = {"font-size": "x-large"})

    expected = dbc.Row("Clouds", className = "weather_row")

    date = from_utc().strftime("%b %d, %Y")
    time = html.B(from_utc().strftime("%H:%M"), style = {"padding-left": "10px"})
    date = dbc.Row([html.Img(src = "assets/calendar_1.png", height = "30px"), date, time], id = "date_row", className = "weather_row", style = {"font-size": "smaller"})
    location = dbc.Row([html.Img(src = "assets/location.png", height = "30px"), "Mont-Saint-Guibert, BE"], className = "weather_row", style = {"font-size": "smaller"})

    return [html.Div([outdoor, indoor, expected, date, location], style = {"position": "absolute", "bottom": "0", "font-size": "large"})]


def make_temperature_plot():
    sensor_element = [dbc.Card(dcc.Graph(
        id = {"type": "other_sensor", "sensor": "428F_94_temp"},
        # config = {'displayModeBar': False},
        animate = True,
        )), dbc.Card(dcc.Graph(
        id = {"type": "other_sensor", "sensor": "bme1_temperature"},
        # config = {'displayModeBar': False},
        animate = True,
        ))]
    return [dbc.Col(c) for c in sensor_element]


def make_highlights():
    mcards = []

    return dbc.Row(dbc.CardDeck([dbc.Col(c) for c in mcards], className = "weather"))

def make_details():
    mcards = []

    latest = mongoClient.get_latest_sensor_value("bme1_humidity")
    mcards.append((dbc.CardImg(src = "assets/humidity_in.png", style = {"width": "50px"}), dbc.CardBody(f"{latest}%")))

    latest = mongoClient.get_latest_sensor_value("428F_94_hum")
    mcards.append((dbc.CardImg(src = "assets/humidity_out.png", style = {"width": "50px"}), dbc.CardBody(f"{latest}%")))

    return dbc.Row(dbc.CardDeck([dbc.Col(c) for c in mcards], className = "weather"))


def get_layout():
    row = [dcc.Interval(id = 'interval-component', interval = 1 * 1000, n_intervals = 0),
        dbc.Row([
            dbc.Col(make_location_summary(), width = 2, style = {"background-image": "url('/assets/weather_bckg.jpg')", "min-height": "400px", "min-width": "250px"}),
            dbc.Col([
                dbc.Row(dbc.Col([dbc.Row("Hourly Temperature"), dbc.Row(make_temperature_plot())])),
                dbc.Row([
                    dbc.Col([dbc.Row("Details"), dbc.Row(dbc.Col(make_details()))]),
                    dbc.Col([dbc.Row("Todays's highlights"), dbc.Row(dbc.Col(make_highlights()))]),
                    ])
                ], style = {"Padding": "50px"})
            ], style = {"background-color": "#1E1E1E", "border-style":"solid"})
        ]
    return row


@app.callback(Output("date_row", 'children'),
              [Input('interval-component', 'n_intervals')],
              [])
def update_date(_):
    date = from_utc().strftime("%b %d, %Y")
    time = html.B(from_utc().strftime("%H:%M"), style = {"padding-left": "10px"})
    return [html.Img(src = "assets/calendar_1.png", height = "23px"), date, time]


@app.callback(Output({"type": "local_value", "sensor": MATCH}, 'children'),
              [Input('interval-component', 'n_intervals')],
              [State({"type": "local_value", "sensor": MATCH}, 'id'), ])
def update_local_value(_, sensor_name):
    val = mongoClient.get_latest_sensor_value(sensor_name["sensor"])
    return val