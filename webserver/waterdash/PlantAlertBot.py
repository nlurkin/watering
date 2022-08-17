#!/bin/env python3

import logging
import sys
import sched
import time
from datetime import datetime

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

def init_bot():
    my_persistence = PicklePersistence(filename='PlantAlertBot')
    updater = Updater(token=telegram_token, persistence=my_persistence)

    dispatcher = updater.dispatcher

    start_handler = CommandHandler('start', start)
    dispatcher.add_handler(start_handler)


    register_handler = CommandHandler('register', register)
    dispatcher.add_handler(register_handler)
    if not "RegisteredUsers" in dispatcher.bot_data:
        dispatcher.bot_data["RegisteredUsers"] = []

    return dispatcher, updater

def check_conditions(owm):
    owm.get_latest_info()
    hourly = owm.prepare_hourly_12h_forecast()

    temps = [(_[1].temperature("celsius")["temp"],_[0]) for _ in hourly]

    message = None
    lower_0 = [_ for _ in temps if _[0]<0]
    higher_30 = [_ for _ in temps if _[0]>30]
    if len(lower_0):
        message = f"WARNING: Temperatures below 0°C foreseen starting at {lower_0[0][1]}"
    if len(higher_30):
        message = f"WARNING: Temperatures above 30°C foreseen starting at {higher_30[0][1]}"

    return message

def inform_clients(updater, dispatcher, message):
    for chat in dispatcher.bot_data["RegisteredUsers"]:
        updater.bot.send_message(chat_id=chat, text=message)

def scheduled_run(s, owm, updater, dispatcher):
    message = check_conditions(owm)
    if message is not None:
        inform_clients(updater, dispatcher, message)

    next_run = datetime.now()
    curr_hour = int(next_run.hour/8 + 1)*8
    next_run = next_run.replace(hour=curr_hour, minute=0, second=0)

    s.enterabs(next_run.timestamp(), 0, lambda : scheduled_run(s, owm, updater, dispatcher))

def main():
    owm = owm_wrapper(owm_token, owm_city)

    dispatcher, updater = init_bot()

    updater.start_polling()

    # Init next run (next multiple of 8h)
    next_run = datetime.now()
    curr_hour = int(next_run.hour/8 + 1)*8
    next_run = next_run.replace(hour=curr_hour, minute=0, second=0)

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
