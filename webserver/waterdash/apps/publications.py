'''
Created on 30-Dec-2020

@author: Nicolas Lurkin
'''

import dash_html_components as html
import dash_bootstrap_components as dbc
from waterapp import mongoClient


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



