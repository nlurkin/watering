'''
Created on 18-Jun-2020

@author: Nicolas Lurkin
'''

import dash
import dash_core_components as dcc
import dash_html_components as html
import dash_bootstrap_components as dbc
from dash.dependencies import Input, Output, State
from waterapp import app, mongoClient
import re


def get_layout():
    return build_list_layout()


def build_list_layout():
    t_header = [html.Thead(html.Tr([html.Th("Device"), html.Th("Name"), html.Th("Type"), html.Th("Online?")]))]

    currentList = mongoClient.get_advertised_current_list()
    allList = mongoClient.get_advertised_all_list()

    currsensor = [_["sensor"] for _ in currentList]
    t_body = [html.Tr([
        html.Td("arduino"), html.Td(sensor['sensor']),
        html.Td(sensor['data-type']), html.Td("Yes" if sensor['sensor'] in currsensor else "No")
        ]) for sensor in allList]

    return [html.Div(dbc.Table(t_header + t_body, dark = True, striped = True))]



