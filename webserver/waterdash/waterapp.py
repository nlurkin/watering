#!/bin/env python
"""
Created on 17 Jun 2020

@author: Nicolas Lurkin
"""

import dash
import dash_bootstrap_components as dbc

from data.config import SECRET_KEY, app_prefix, mongo_creds, owm_city, owm_token
from mongodb import myMongoClient
from owm import owm_wrapper

# Initialise the app
app = dash.Dash(
    __name__,
    external_stylesheets=[dbc.themes.BOOTSTRAP],
    suppress_callback_exceptions=True,
    requests_pathname_prefix=f"{app_prefix}/",
)
server = app.server

server.config.update(SECRET_KEY=SECRET_KEY)

mongoClient = myMongoClient(
    mongo_creds["server"],
    27017,
    mongo_creds["app_name"],
    mongo_creds["username"],
    mongo_creds["password"],
)
mongoClient.connect()

owm = owm_wrapper(owm_token, owm_city)
