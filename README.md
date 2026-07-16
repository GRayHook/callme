CallMe
======

About
-----

  This utility can send notify to telegram chat when command is done.

  My bot is @CommandNotifyBot.

Requirements:
-------------

+ json-c
+ curl

Setup
-----

The Telegram bot token and (optionally) a proxy URL are injected at compile
time from a gitignored `.env` file, not stored in source:

    cp .env.example .env
    # edit .env: TG_TOKEN=<your bot token>, TG_PROXY=<proxy or empty>

Then `make`. `TG_PROXY` may be left empty to disable the proxy. Both values
can also be overridden on the command line: `make TG_TOKEN=... TG_PROXY=...`.

Features
--------

+ callme -n
  Write your chat id to config and exit.

+ callme -s 'some script'
  Execute script via /bin/bash.

+ callme -e /some/program --and --args
  Execute some bin with gived args.
