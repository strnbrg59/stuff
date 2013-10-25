#!/bin/sh

func1() {
    echo "func1"
}
func2() {
    echo "func2"
}

FUNCTION=$1
case "${FUNCTION}" in
    func1 | func2)
        $FUNCTION ;;
esac
