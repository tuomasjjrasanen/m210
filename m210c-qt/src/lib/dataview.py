from __future__ import absolute_import
from __future__ import division

import functools

from PyQt4.QtCore import Qt
from PyQt4.QtCore import QRectF
from PyQt4.QtCore import QSizeF
from PyQt4.QtCore import QByteArray

from PyQt4.QtGui import QPainter
from PyQt4.QtGui import QWidget
from PyQt4.QtGui import QVBoxLayout

from PyQt4.QtSvg import QSvgWidget

from m210.qt.ui.dataview import Ui_DataView

import m210.pegatech

class DataView(QWidget, Ui_DataView):

    def __init__(self, *args, **kwargs):
        QWidget.__init__(self, *args, **kwargs)
        self.setupUi(self)
        self.__data = None
        self.__notes = []
        self.spinBox.valueChanged.connect(self.note_changed)
        self.__svg_view = QSvgWidget(self)
        layout = QVBoxLayout()
        layout.addWidget(self.__svg_view)
        self.groupBox.setLayout(layout)

    def note_changed(self, note_number):
        if self.__notes:
            # note_number can be zero when the maximum is set to
            # zero. That would lead to invalid index.
            svg = self.__notes[note_number-1].as_svg()
            self.__svg_view.load(QByteArray(svg))

    def data(self):
        return self.__data

    def clear_data(self):
        self.set_data(None)
        self.__svg_view.load(QByteArray(""))

    def set_data(self, data):
        self.__data = data
        if data:
            self.setEnabled(True)
            self.__notes = list(m210.pegatech.note_iter_from_data(data))
            note_count = len(self.__notes)
            self.horizontalSlider.setMaximum(note_count)
            self.horizontalSlider.setMinimum(1)
            self.spinBox.setMaximum(note_count)
            self.spinBox.setMinimum(1)
        else:
            self.__notes = []
            self.setEnabled(False)
            self.horizontalSlider.setMaximum(0)
            self.spinBox.setMaximum(0)
            # self.graphicsView.scene().clear()

if __name__ == "__main__":
    import sys
    from PyQt4.QtGui import QApplication
    app = QApplication(sys.argv)
    view = DataView()
    data = open(sys.argv[1], "rb").read()
    view.set_data(data)
    view.show()
    sys.exit(app.exec_())
