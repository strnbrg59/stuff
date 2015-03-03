DEBUG_BIN = bin/CardationCLI-debug.apk
RELEASE_BIN = bin/CardationCLI-release.apk

debug : debug_build install

debug_build :
	ant debug

release :
	ant release

install :
	adb install -r $(DEBUG_BIN)

clean :
	ant clean
