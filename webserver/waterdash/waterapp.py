#!/bin/env python
'''
Created on 17 Jun 2020

@author: Nicolas Lurkin
'''

import dash
import dash_bootstrap_components as dbc
from mongodb import myMongoClient
from data.config import mongo_creds, app_prefix

# Initialise the app
app = dash.Dash(__name__, external_stylesheets = [dbc.themes.BOOTSTRAP],
                suppress_callback_exceptions = True,
                requests_pathname_prefix = f"{app_prefix}/")
server = app.server

mongoClient = myMongoClient(mongo_creds["server"], 27017, mongo_creds["username"], mongo_creds["password"])
mongoClient.connect()
