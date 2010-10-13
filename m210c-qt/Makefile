.PHONY = all ui clean

UI_DIR=ui/
UI_FILES=$(wildcard $(UI_DIR)*.ui)
UI_MODULE_DIR=src/lib/ui/
UI_MODULES=$(addprefix $(UI_MODULE_DIR), $(notdir $(patsubst %.ui, %.py, $(UI_FILES))))

all: ui

$(UI_MODULES): $(UI_MODULE_DIR)%.py: $(UI_DIR)%.ui
	pyuic4 $< > $@

ui: uidir $(UI_MODULES)
	pyrcc4 ui/noteviewer.qrc > src/lib/ui/noteviewer_rc.py

uidir:
	mkdir -p $(UI_MODULE_DIR)
	touch $(UI_MODULE_DIR)__init__.py

clean:
	rm -rf $(UI_MODULE_DIR)
