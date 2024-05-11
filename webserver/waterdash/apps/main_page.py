"""
Created on 21 Mar 2021

@author: Nicolas Lurkin
"""

import datetime

import dash_bootstrap_components as dbc
import numpy as np
import pandas as pd
import plotly.graph_objects as go
from dash import dcc, html
from dash.dependencies import MATCH, Input, Output, State
from pyowm.utils import timestamps

from mongodb import from_utc, to_utc
from waterapp import app, mongoClient, owm


def make_location_summary():
    latest = mongoClient.get_latest_sensor_value("bme1_temperature")
    indoor = dbc.Row(
        [
            html.Img(src="assets/in_house.png", className="weather-img-logo"),
            html.Div(latest, id={"type": "local_value", "sensor": "bme1_temperature"}),
            "\u00B0C",
        ],
        className="weather_row",
        style={"font-size": "larger"},
    )
    latest = mongoClient.get_latest_sensor_value("428F_B3_temp")
    outdoor = dbc.Row(
        [
            html.Img(src="assets/out_house.png", className="weather-img-logo"),
            html.Div(latest, id={"type": "local_value", "sensor": "428F_B3_temp"}),
            "\u00B0C",
        ],
        className="weather_row",
        style={"font-size": "larger"},
    )

    obs = owm.get_latest_info()["obs"]
    expected = dbc.Row(
        [
            html.Img(
                src=obs.weather_icon_url(),
                className="weather-img-logo",
            ),
            obs.detailed_status,
        ],
        className="weather_row",
    )

    date = from_utc().strftime("%b %d, %Y")
    time = html.B(from_utc().strftime("%H:%M"), style={"padding-left": "10px"})
    date = html.Div(
        [
            html.Img(
                src="assets/calendar_1.png",
                className="weather-img-logo",
                style={"padding-left": "10px"},
            ),
            date,
            time,
        ],
        id="date_row",
        className="weather_row",
        style={"font-size": "smaller", "margin": "5px"},
    )
    location = dbc.Row(
        [
            html.Img(src="assets/location.png", className="weather-img-logo"),
            "Mont-Saint-Guibert, BE",
        ],
        className="weather_row",
        style={"font-size": "smaller"},
    )

    return [
        html.Div(
            [outdoor, indoor, expected, date, location],
            style={"position": "relative", "top": "250px", "font-size": "x-large"},
        )
    ]


def make_temperature_plot():
    sensor_element = [
        dbc.Card(
            dcc.Graph(
                id={"type": "other_sensor", "sensor": "428F_B3_temp"},
                # config = {'displayModeBar': False},
                animate=True,
            )
        ),
        dbc.Card(
            dcc.Graph(
                id={"type": "other_sensor", "sensor": "bme1_temperature"},
                # config = {'displayModeBar': False},
                animate=True,
            )
        ),
    ]
    return [dbc.Col(c) for c in sensor_element]


def make_forecast():
    mcards = []

    forecaster = owm.get_latest_info()["forecast"]
    forecaster.forecast.actualize()
    # Now remove everything from more than 24h in the future
    next24h = timestamps.tomorrow()
    actualized_weathers = filter(
        lambda x: x.reference_time(timeformat="date") < next24h,
        forecaster.forecast.weathers,
    )
    forecaster.forecast.weathers = list(actualized_weathers)

    vmin = forecaster.most_cold().temperature("celsius")["temp"]
    vmax = forecaster.most_hot().temperature("celsius")["temp"]
    mcards.append(
        (
            dbc.CardImg(src="assets/out_house.png", style={"width": "50px"}),
            dbc.CardBody(
                [html.P(f"Max: {vmax}\u00B0C"), html.P(f"Min: {vmin}\u00B0C")]
            ),
        )
    )

    vmax = forecaster.most_windy().wind()["speed"] * 3.6  # In km/h
    vgusts = max(_.wind()["gust"] * 3.6 for _ in forecaster.forecast)  # In km/h
    mcards.append(
        (
            dbc.CardImg(src="assets/wind.png", style={"width": "50px"}),
            dbc.CardBody(
                [html.P(f"Max: {vmax:.0f} km/h"), html.P(f"Gusts: {vgusts:.0f} km/h")]
            ),
        )
    )

    return dbc.Row(html.Div([dbc.Col(c, width=5) for c in mcards], className="weather"))


def make_highlights():
    mcards = []

    values = get_and_merge_data("bme1_temperature", 24)
    vmax = values["val"].max()
    vmin = values["val"].min()
    mcards.append(
        (
            dbc.CardImg(src="assets/in_house.png", style={"width": "50px"}),
            dbc.CardBody(
                [html.P(f"Max: {vmax}\u00B0C"), html.P(f"Min: {vmin}\u00B0C")]
            ),
        )
    )

    values = get_and_merge_data("bme1_humidity", 24)
    vmax = values["val"].max()
    vmin = values["val"].min()
    mcards.append(
        (
            dbc.CardImg(src="assets/humidity_in.png", style={"width": "50px"}),
            dbc.CardBody([html.P(f"Max: {vmax} %"), html.P(f"Min: {vmin} %")]),
        )
    )

    values = get_and_merge_data("428F_B3_temp", 24)
    vmax = values["val"].max()
    vmin = values["val"].min()
    mcards.append(
        (
            dbc.CardImg(src="assets/out_house.png", style={"width": "50px"}),
            dbc.CardBody(
                [html.P(f"Max: {vmax}\u00B0C"), html.P(f"Min: {vmin}\u00B0C")]
            ),
        )
    )

    values = get_and_merge_data("428F_B3_hum", 24)
    vmax = values["val"].max()
    vmin = values["val"].min()
    mcards.append(
        (
            dbc.CardImg(src="assets/humidity_out.png", style={"width": "50px"}),
            dbc.CardBody([html.P(f"Max: {vmax} %"), html.P(f"Min: {vmin} %")]),
        )
    )

    return dbc.Row(html.Div([dbc.Col(c, width=5) for c in mcards], className="weather"))


def make_details():
    mcards = []

    latest = mongoClient.get_latest_sensor_value("bme1_humidity")
    mcards.append(
        (
            dbc.CardImg(src="assets/humidity_in.png", style={"width": "50px"}),
            dbc.CardBody(f"{latest} %"),
        )
    )

    latest = mongoClient.get_latest_sensor_value("428F_B3_hum")
    mcards.append(
        (
            dbc.CardImg(src="assets/humidity_out.png", style={"width": "50px"}),
            dbc.CardBody(f"{latest} %"),
        )
    )

    latest = mongoClient.get_latest_sensor_value("bme1_pressure")
    values = get_and_merge_data("bme1_pressure", 24)
    values = values.asof(to_utc() - datetime.timedelta(hours=6))["val"]
    updown = (
        "stable"
        if np.abs(latest - values) < 1
        else ("up" if (latest - values > 0) else "down")
    )
    mcards.append(
        (
            dbc.CardImg(
                src=f"assets/barometer_in_{updown}.png", style={"width": "50px"}
            ),
            dbc.CardBody(html.P(f"{latest} mm Hg")),
        )
    )

    obs = owm.get_latest_info()["obs"]

    feels = obs.temperature("celsius")["feels_like"]
    mcards.append(
        (
            dbc.CardImg(src="assets/feel.png", style={"width": "50px"}),
            dbc.CardBody(f"{feels}\u00B0C"),
        )
    )

    wind = obs.wind()["speed"] * 3.6  # In km/h
    gust = "N/A"
    if "gust" in obs.wind():
        gust = f"{obs.wind()['gust']*3.6:.0f} km/h"  # In km/h
    mcards.append(
        (
            dbc.CardImg(src="assets/wind.png", style={"width": "50px"}),
            dbc.CardBody([html.P(f"{wind:.0f} km/h"), html.P(f"Gusts: {gust}")]),
        )
    )

    rain = obs.rain
    if len(rain) == 0:
        rain = "N/A"
    else:
        rain = f"{rain['1h']} mm"
    mcards.append(
        (
            dbc.CardImg(src="assets/rain_level.png", style={"width": "50px"}),
            dbc.CardBody(f"{rain}"),
        )
    )

    return dbc.Row(html.Div([dbc.Col(c, width=5) for c in mcards], className="weather"))


def get_layout():
    row = [
        dcc.Interval(id="interval-component", interval=300 * 1000, n_intervals=0),
        dbc.Row(
            [
                dbc.Col(
                    make_location_summary(),
                    width=2,
                    style={
                        "background-image": "url('assets/weather_bckg.jpg')",
                        "min-height": "400px",
                        "max-height": "730px",
                        "min-width": "290px",
                    },
                ),
                dbc.Col(
                    [
                        dbc.Row(
                            dbc.Col(
                                [
                                    dbc.Row(
                                        "Hourly Temperature",
                                        style={"font-size": "large"},
                                    ),
                                    dbc.Row(make_temperature_plot()),
                                ]
                            )
                        ),
                        dbc.Row(
                            [
                                dbc.Col(
                                    [
                                        dbc.Row(
                                            "Details", style={"font-size": "large"}
                                        ),
                                        dbc.Row(
                                            dbc.Col(make_details()), justify="start"
                                        ),
                                    ]
                                ),
                                dbc.Col(
                                    [
                                        dbc.Row(
                                            "24h highlights",
                                            style={"font-size": "large"},
                                        ),
                                        dbc.Row(
                                            dbc.Col(make_highlights()), justify="start"
                                        ),
                                        dbc.Row(
                                            "24h forecast", style={"font-size": "large"}
                                        ),
                                        dbc.Row(
                                            dbc.Col(make_forecast()), justify="start"
                                        ),
                                    ]
                                ),
                            ]
                        ),
                    ],
                    style={"Padding": "50px"},
                ),
            ],
            style={"background-color": "#1E1E1E", "border-style": "solid"},
        ),
    ]
    return row


@app.callback(
    Output("date_row", "children"), [Input("interval-component", "n_intervals")], []
)
def update_date(_):
    date = from_utc().strftime("%b %d, %Y")
    time = html.B(from_utc().strftime("%H:%M"), style={"padding-left": "10px"})
    return [
        html.Img(
            src="assets/calendar_1.png",
            style={"height": "23px", "padding-left": "10px"},
            className="weather-img-logo",
        ),
        date,
        time,
    ]


@app.callback(
    Output({"type": "local_value", "sensor": MATCH}, "children"),
    [Input("interval-component", "n_intervals")],
    [
        State({"type": "local_value", "sensor": MATCH}, "id"),
    ],
)
def update_local_value(_, sensor_name):
    val = mongoClient.get_latest_sensor_value(sensor_name["sensor"])
    return val


def get_and_merge_data(sensor_name, nhours):
    start_time = to_utc() - datetime.timedelta(hours=nhours)
    end_time = to_utc()
    value_doc = mongoClient.get_all_sensor_values(
        sensor_name, start_time.strftime("%Y-%m-%d"), end_time.strftime("%Y-%m-%d")
    )
    values = []
    for doc in value_doc:
        values.extend(doc["samples"])
    if len(values) == 0:
        value_doc = None
    else:
        value_doc = {"samples": values}

    if value_doc is None:
        df = pd.DataFrame({"val": []})
    else:
        if "samples" not in value_doc:
            df = pd.DataFrame({"val": []})
        else:
            df = pd.DataFrame(value_doc["samples"])
            df.index = pd.to_datetime(df["ts"], unit="s", utc=True)
        new_index = to_utc()
        df = pd.concat(
            [
                df,
                pd.DataFrame(
                    index=[new_index], data=df.tail(1).values, columns=df.columns
                ),
            ]
        )

    df = df.loc[start_time.strftime("%Y-%m-%d %H:%M") :]

    if len(df) > 0:
        df.index = df.index.tz_convert("Europe/Brussels")

    return df


@app.callback(
    Output({"type": "other_sensor", "sensor": MATCH}, "figure"),
    [Input("interval-component", "n_intervals")],
    [
        State({"type": "other_sensor", "sensor": MATCH}, "id"),
    ],
)
def update_float_metrics(_, sensor_name):
    title = {"bme1_temperature": "Indoors", "428F_B3_temp": "Outdoors"}

    dfo = get_and_merge_data(sensor_name["sensor"], 12)
    df = dfo.resample("1h").first()
    df.loc[dfo.iloc[-1].name] = dfo.iloc[-1]

    figure = go.Figure().add_trace(
        go.Scatter(x=df.index, y=df["val"], mode="lines+markers", name="Read")
    )
    images = []
    if title[sensor_name["sensor"]] == "Outdoors":
        hourly = owm.prepare_hourly_12h_forecast()
        df_forecast = pd.DataFrame().reindex_like(df)
        df_forecast.loc[dfo.iloc[-1].name] = dfo.iloc[-1]
        for i, (h, w) in enumerate(hourly):
            temp = w.temperature("celsius")["temp"]
            df_forecast.loc[h] = {"val": temp, "ts": h}
            images.append((i, temp, w.weather_icon_url()))
        figure = figure.add_trace(
            go.Scatter(
                x=df_forecast.index,
                y=df_forecast["val"],
                mode="lines+markers",
                name="Forecast",
            )
        )

    for x, y, img in images:
        figure.add_layout_image(
            go.layout.Image(
                source=img,
                xref="paper",
                yref="y",
                x=0.625 + x / 9,
                y=y,
                sizex=3,
                sizey=3,
                xanchor="center",
                yanchor="middle",
            )
        )

    figure.update_layout(
        title=dict(
            text=title[sensor_name["sensor"]],
            x=0.5,
        ),
        margin_t=30,
        margin_b=10,
        margin_l=0,
        margin_r=0,
        height=200,
        yaxis_title="\u00B0C",
        showlegend=False,
        # xaxis = xaxis,
        # template = "plotly_dark",
    )

    return figure
