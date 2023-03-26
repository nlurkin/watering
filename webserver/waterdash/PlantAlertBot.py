#!/bin/env python3

import logging
import sys
import sched
import time
from datetime import datetime, timedelta

from telegram import Update
from telegram.ext import (CallbackContext, CommandHandler, PicklePersistence,
                          Updater)
from owm import owm_wrapper
from data.config import owm_token, owm_city, telegram_token

logging.basicConfig(format='%(asctime)s - %(name)s - %(levelname)s - %(message)s', level=logging.INFO)

def start(update: Update, context: CallbackContext):
    context.bot.send_message(chat_id=update.effective_chat.id, text="I'm a bot, please talk to me!")


def add_user(context, chat_id):
    if not "RegisteredUsers" in context.bot_data:
        context.bot_data["RegisteredUsers"] = []
    context.bot_data["RegisteredUsers"].append(chat_id)

def register(update: Update, context: CallbackContext):
    chat_id = update.effective_chat.id
    add_user(context, chat_id)
    context.bot.send_message(chat_id=update.effective_chat.id, text="Thank you for registering")

def am_i_registered(update: Update, context: CallbackContext):
    chat_id = update.effective_chat.id
    if chat_id in context.bot_data["RegisteredUsers"]:
        context.bot.send_message(chat_id=update.effective_chat.id, text="I'm not forgetting about you!")
    else:
        context.bot.send_message(chat_id=update.effective_chat.id, text="Sorry, who are you?")


def init_bot():
    my_persistence = PicklePersistence(filename='PlantAlertBot')
    updater = Updater(token=telegram_token, persistence=my_persistence)

    dispatcher = updater.dispatcher

    handler = CommandHandler('start', start)
    dispatcher.add_handler(handler)

    handler = CommandHandler('register', register)
    dispatcher.add_handler(handler)

    handler = CommandHandler('amiregistered', am_i_registered)
    dispatcher.add_handler(handler)

    if not "RegisteredUsers" in dispatcher.bot_data:
        dispatcher.bot_data["RegisteredUsers"] = []

    return dispatcher, updater

def check_value_alerts(values, low_levels, high_levels, val_name, unit):
    for l in low_levels:
        alerts = [_ for _ in values if _[0]<l]
        if len(alerts)>0:
            return f"WARNING: {val_name} below {l}{unit} foreseen starting at {alerts[0][1]}"

    for l in high_levels:
        alerts = [_ for _ in values if _[0]>l]
        if len(alerts)>0:
            return f"WARNING: {val_name} above {l}{unit} foreseen starting at {alerts[0][1]}"

    return None


def check_conditions(owm):
    owm.get_latest_info()
    hourly = owm.prepare_hourly_12h_forecast()
    message_list = []

    temps = [(_[1].temperature("celsius")["temp"],_[0]) for _ in hourly]
    message = check_value_alerts(temps, [0], [30, 20], "Temperature", "°C")
    if message is not None:
        message_list.append(message)

    message = None
    lower_0 = [_ for _ in temps if _[0]<0]
    higher_30 = [_ for _ in temps if _[0]>30]
    higher_20 = [_ for _ in temps if _[0]>20]
    if len(lower_0):
        message = f"WARNING: Temperatures below 0°C foreseen starting at {lower_0[0][1]}"
    if len(higher_20):
        message = f"WARNING: Temperatures above 20°C foreseen starting at {higher_20[0][1]}"
    if len(higher_30):
        message = f"WARNING: Temperatures above 30°C foreseen starting at {higher_30[0][1]}"

    return message_list

def inform_clients(updater, dispatcher, message):
    for chat in dispatcher.bot_data["RegisteredUsers"]:
        updater.bot.send_message(chat_id=chat, text=message)

def scheduled_run(s, owm, updater, dispatcher):
    message_list = check_conditions(owm)
    for message in message_list:
        inform_clients(updater, dispatcher, message)

    next_run = datetime.now()
    curr_hour = int(next_run.hour/8)*8
    next_run = next_run.replace(hour=curr_hour, minute=0, second=0) + timedelta(hours=8)

    s.enterabs(next_run.timestamp(), 0, lambda : scheduled_run(s, owm, updater, dispatcher))

def main():
    owm = owm_wrapper(owm_token, owm_city)

    dispatcher, updater = init_bot()

    updater.start_polling()

    # Init next run (next multiple of 8h)
    next_run = datetime.now()
    curr_hour = int(next_run.hour/8)*8
    next_run = next_run.replace(hour=curr_hour, minute=0, second=0) + timedelta(hours=8)

    s = sched.scheduler(time.time, time.sleep)
    s.enterabs(next_run.timestamp(), 0, lambda : scheduled_run(s, owm, updater, dispatcher))
    while True:
        try:
            s.run()
        except KeyboardInterrupt:
            break

    return 0

if __name__ == "__main__":
    sys.exit(main())
