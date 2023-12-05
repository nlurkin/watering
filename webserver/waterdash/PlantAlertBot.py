#!/bin/env python3

import logging
import sys
import time
from datetime import time

import pytz
from telegram import Update
from telegram.ext import (ApplicationBuilder, CallbackContext, CommandHandler,
                          JobQueue, PicklePersistence)

from data.config import owm_city, owm_token, telegram_token
from owm import owm_wrapper

logging.basicConfig(
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s', level=logging.INFO)


###########################
# User management functions
###########################
def ensure_registered_users(context):
    if "RegisteredUsers" not in context.bot_data:
        context.bot_data["RegisteredUsers"] = []


def add_user(context, chat_id):
    ensure_registered_users(context)
    context.bot_data["RegisteredUsers"].append(chat_id)


##############
# Bot commands
##############
async def start(update: Update, context: CallbackContext):
    await context.bot.send_message(chat_id=update.effective_chat.id, text="I'm a bot, please talk to me!")


async def register(update: Update, context: CallbackContext):
    chat_id = update.effective_chat.id
    add_user(context, chat_id)
    await context.bot.send_message(
        chat_id=update.effective_chat.id, text="Thank you for registering")


async def am_i_registered(update: Update, context: CallbackContext):
    ensure_registered_users(context)
    chat_id = update.effective_chat.id
    if chat_id in context.bot_data["RegisteredUsers"]:
        await context.bot.send_message(
            chat_id=update.effective_chat.id, text="I'm not forgetting about you!")
    else:
        await context.bot.send_message(
            chat_id=update.effective_chat.id, text="Sorry, who are you?")


async def inform_clients(context: CallbackContext, message: str):
    ensure_registered_users(context)
    for chat in context.bot_data["RegisteredUsers"]:
        await context.bot.send_message(chat_id=chat, text=message)


################
# Initialization
################
def init_bot():
    my_persistence = PicklePersistence(filepath='PlantAlertBot')
    application = ApplicationBuilder().token(
        telegram_token).persistence(my_persistence).build()

    handler = CommandHandler('start', start)
    application.add_handler(handler)

    handler = CommandHandler('register', register)
    application.add_handler(handler)

    handler = CommandHandler('amiregistered', am_i_registered)
    application.add_handler(handler)

    return application


async def start_bot(application):
    await application.initialize()
    await application.start()

    if "RegisteredUsers" not in application.bot_data:
        application.bot_data["RegisteredUsers"] = []

    await application.updater.start_polling()


#####################
# Open Weather checks
#####################
def check_value_alerts(values, low_levels, high_levels, val_name, unit):
    min_val = min([_[0] for _ in values])
    max_val = max([_[0] for _ in values])
    for l in low_levels:
        alerts = [_ for _ in values if _[0] < l]
        bxl_time = alerts[0][1].astimezone(pytz.timezone('Europe/Brussels'))
        if len(alerts) > 0:
            return f"WARNING: {val_name} below {l}{unit} foreseen starting at {bxl_time}, with a minimum of {min_val}"

    for l in high_levels:
        alerts = [_ for _ in values if _[0] > l]
        if len(alerts) > 0:
            bxl_time = alerts[0][1].astimezone(
                pytz.timezone('Europe/Brussels'))
            return f"WARNING: {val_name} above {l}{unit} foreseen starting at {bxl_time}, with a maximum of {max_val}"

    return None


def check_conditions(owm):
    owm.get_latest_info()
    hourly = owm.prepare_hourly_12h_forecast()
    message_list = []

    temps = [(_[1].temperature("celsius")["temp"], _[0]) for _ in hourly]
    message = check_value_alerts(temps, [5], [30, 20], "Temperature", "°C")
    if message is not None:
        message_list.append(message)

    gusts = [(_[1].wind()["gust"]*3.6, _[0]) for _ in hourly]  # In km/h
    message = check_value_alerts(gusts, [], [60, 80], "Wind gusts", "kph")
    if message is not None:
        message_list.append(message)

    return message_list


async def scheduled_run(context):
    owm = context.job.data['owm']

    message_list = check_conditions(owm)
    for message in message_list:
        await inform_clients(context, message)


def init_owm_job(job_queue: JobQueue):
    from mongodb import from_utc, to_utc
    owm = owm_wrapper(owm_token, owm_city)
    for i in range(0, 10, 1):
        job_queue.run_daily(scheduled_run, time=time(
            23, i, 0, tzinfo=pytz.timezone('Europe/Brussels')), data={'owm': owm})


def main():

    application = init_bot()

    init_owm_job(application.job_queue)

    application.run_polling()

    return 0


if __name__ == "__main__":
    sys.exit(main())
