#! /usr/bin/env python

# demo.py --- A simple demonstration program for pythondialog
# Copyright (C) 2000  Robb Shecter, Sultanbek Tezadov
# Copyright (C) 2002, 2004  Florent Rougon
#
# This program is in the public domain.

"""Demonstration program for pythondialog.

This is a simple program demonstrating the possibilities offered by
the pythondialog module (which is itself a Python interface to the
well-known dialog utility, or any other program compatible with
dialog).

Please have a look at the documentation for the `handle_exit_code'
function in order to understand the somewhat relaxed error checking
policy for pythondialog calls in this demo.

"""

import sys, os, os.path, time, string, dialog

FAST_DEMO = 0

d = dialog.Dialog (dialog="dialog")

# XXX We should handle the new DIALOG_HELP and DIALOG_EXTRA return codes here.
def handle_exit_code(d, code):
    """Sample function showing how to interpret the dialog exit codes.

    This function is not used after every call to dialog in this demo
    for two reasons:

       1. For some boxes, unfortunately, dialog returns the code for
          ERROR when the user presses ESC (instead of the one chosen
          for ESC). As these boxes only have an OK button, and an
          exception is raised and correctly handled here in case of
          real dialog errors, there is no point in testing the dialog
          exit status (it can't be CANCEL as there is no CANCEL
          button; it can't be ESC as unfortunately, the dialog makes
          it appear as an error; it can't be ERROR as this is handled
          in dialog.py to raise an exception; therefore, it *is* OK).

       2. To not clutter simple code with things that are
          demonstrated elsewhere.

    """
    # d is supposed to be a Dialog instance
    if code in (d.DIALOG_CANCEL, d.DIALOG_ESC):
        if code == d.DIALOG_CANCEL:
            msg = "You chose cancel in the last dialog box. Do you want to " \
                  "exit this application?"
        else:
            msg = "You pressed ESC in the last dialog box. Do you want to " \
                  "exit this application?"
        # "No" or "ESC" will bring the user back to the demo.
        # DIALOG_ERROR is propagated as an exception and caught in main().
        # So we only need to handle OK here.
        if d.yesno(msg) == d.DIALOG_OK:
            sys.exit(0)
        return 0
    else:
        return 1                        # code is d.DIALOG_OK
        

def ui_info(msg):
    # Exit code thrown away to keey this demo code simple (however, real
    # errors are propagated by an exception)
    d.infobox (msg)


def gauge_demo(d):
    d.gauge_start("Progress: 0%", title="Still testing your patience...")
    for i in range(1, 101):
	if i < 50:
	    d.gauge_update(i, "Progress: %d%%" % i, update_text=1)
	elif i == 50:
	    d.gauge_update(i, "Over %d%%. Good." % i, update_text=1)
	elif i == 80:
	    d.gauge_update(i, "Yeah, this boring crap will be over Really "
                           "Soon Now.", update_text=1)
	else:
            d.gauge_update(i)

        if FAST_DEMO:
            time.sleep(0.01)
        else:
            time.sleep(0.1)
    d.gauge_stop()
    

def ui_yesno(question):
    # Return the answer given to the question (also specifies if ESC was
    # pressed)
    return d.yesno(question)
    

def ui_msg (msg):
    d.msgbox(msg)


def ui_text(file):
    d.textbox(file, width=76)


def ui_input(msg,init_str=""):
    # If the user presses Cancel, he is asked (by handle_exit_code) if he
    # wants to exit the demo. We loop as long as he tells us he doesn't want
    # to do so.
    while 1:
        (code, answer) = d.inputbox(msg, init=init_str)
        if handle_exit_code(d, code):
            break
    return answer


def ui_menu(question, my_choices):
    while 1:
        (code, tag) = d.menu(
            question,
            width=60,
            choices=my_choices)
        if handle_exit_code(d, code):
            break
    return tag



def ui_passwordbox(message):
    while 1:
        (code, password) = d.passwordbox(message)
        if handle_exit_code(d, code):
            break
    return password





