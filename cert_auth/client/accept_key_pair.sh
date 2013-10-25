#!/bin/sh
. ./common.sh

post_data="public=This%20is%20the%20CMC's%20(public)%20certificate%0A&private=This%20is%20the%20CMC's%20private%20key%0A"

curl --data $post_data $server/accept_key_pair/
