#!/bin/sh
. ./common.sh

steelhead=D21ABCDABCD
post_data="name=${steelhead}&cert=This%20is%20Steelhead%20${steelhead}'s%20certificate%0A"
curl --data $post_data $server/sign_certificate/
