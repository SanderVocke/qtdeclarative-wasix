CONFIG += testcase
TARGET = tst_qqmlqt
SOURCES += tst_qqmlqt.cpp

include (../../shared/util.pri)

macx:CONFIG -= app_bundle

TESTDATA = data/*

QT += core-private v8-private qml-private quick-private testlib gui gui-private
