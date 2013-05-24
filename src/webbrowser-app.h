/*
 * Copyright 2013 Canonical Ltd.
 *
 * This file is part of webbrowser-app.
 *
 * webbrowser-app is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * webbrowser-app is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __WEBBROWSER_APP_H__
#define __WEBBROWSER_APP_H__

#include <QtWidgets/QApplication>

class QQuickView;

class CommandLineParser;
class HistoryModel;
class HistoryMatchesModel;

class WebBrowserApp : public QApplication
{
    Q_OBJECT

public:
    WebBrowserApp(int& argc, char** argv);
    ~WebBrowserApp();

    bool initialize();
    int run();

private Q_SLOTS:
    void onTitleChanged();

private:
    QQuickView* m_view;
    CommandLineParser* m_arguments;
    HistoryModel* m_history;
    HistoryMatchesModel* m_historyMatches;
};

#endif // __WEBBROWSER_APP_H__