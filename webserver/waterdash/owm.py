'''
Created on 25 Mar 2021

@author: nlurkin
'''

from pyowm import OWM
from mongodb import to_utc, from_utc
from pyowm.utils import timestamps


class owm_wrapper():

    def __init__(self, key, city_id):
        self.owm = OWM(key)
        self.owm_mgr = self.owm.weather_manager()
        self.city_id = city_id
        self.last_update = None
        self.owm_object = {}

    def __do_update(self):
        self.last_update = to_utc()

        self.owm_object["obs"] = self.owm_mgr.weather_at_id(self.city_id).weather
        self.owm_object["forecast"] = self.owm_mgr.forecast_at_id(self.city_id, '3h')

    def get_latest_info(self):
        if self.last_update is None or (to_utc() - self.last_update).total_seconds() > 60 * 15:
            self.__do_update()

        return self.owm_object

    def prepare_hourly_12h_forecast(self):
        hours = []
        curr = from_utc()
        curr = curr.replace(minute = 0, second = 0)
        for _ in range(4):
            hours.append(timestamps.next_three_hours(curr))
            curr = hours[-1]

        return [(h, self.owm_object["forecast"].get_weather_at(h)) for h in hours]
