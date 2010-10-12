.PHONY = all clean

all: src/lib/ui/mainwindow.py

src/lib/ui/mainwindow.py: ui/mainwindow.ui
	pyuic4 $^ > $@

clean:
	rm src/lib/ui/mainwindow.py
