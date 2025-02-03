#!/bin/bash

curl -sL 'https://api.privatbank.ua/p24api/pubinfo?json&exchange&coursid=5' | jq -r '.[] | select(.ccy == "USD") | "USD: \(.buy | tonumber | . * 1.0 | tostring | .[0:5]), \(.sale | tonumber | . * 1.0 | tostring | .[0:5])"'
