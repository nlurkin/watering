#!/bin/env python
'''
Created on 17 Jun 2020

@author: Nicolas Lurkin
'''

import dash
import dash_bootstrap_components as dbc
from mongodb import myMongoClient

# Initialise the app
app = dash.Dash(__name__, external_stylesheets = [dbc.themes.BOOTSTRAP], suppress_callback_exceptions = True)
server = app.server

mongoClient = myMongoClient("192.168.0.18", 27017)
mongoClient.connect()
