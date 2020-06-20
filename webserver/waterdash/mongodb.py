'''
Created on 18-Jun-2020

@author: Nicolas Lurkin
'''

import pymongo


class myMongoClient(object):

    def __init__(self, hostname, port):
        self.hostname = hostname
        self.port = port
        self.client = None
        self.sensors_db = None
        self.dashboard_db = None

    def connect(self):
        self.client = pymongo.MongoClient(self.hostname, self.port)

        self.sensors_db = self.client["sensors"]["definition"]
        self.dashboard_db = self.client["dashboard"]["dashboard_config"]

    def close(self):
        self.client.close()
