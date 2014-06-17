/*
 * Copyright 2014 Canonical Ltd.
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

// Qt
#include <QtCore/QDir>
#include <QtCore/QSet>
#include <QtCore/QTemporaryDir>
#include <QtNetwork/QNetworkCookie>
#include <QtTest/QSignalSpy>
#include <QtTest/QtTest>

// local
#include "chrome-cookie-store.h"
#include "webkit-cookie-store.h"

uint qHash(const QNetworkCookie &cookie, uint seed)
{
    return qHash(cookie.toRawForm(), seed);
}

class CookieStoreTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testChromeProperties();
    void testWebkitProperties();

    void testChromeReadWrite_data() { setupCookieData(); }
    void testWebkitReadWrite_data() { setupCookieData(); }
    void testChromeReadWrite();
    void testWebkitReadWrite();

    void testMoving_data() { setupCookieData(); }
    void testMoving();

private:
    void setupCookieData();
    QSet<QNetworkCookie> parseCookies(const Cookies &rawCookies);
};

QSet<QNetworkCookie>
CookieStoreTest::parseCookies(const Cookies &rawCookies)
{
    QList<QNetworkCookie> cookies;
    Q_FOREACH(const QByteArray &rawCookie, rawCookies) {
        cookies.append(QNetworkCookie::parseCookies(rawCookie));
    }
    return cookies.toSet();
}

void CookieStoreTest::testChromeProperties()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QDir testDir(tmpDir.path());
    QTemporaryDir tmpDir2;
    QVERIFY(tmpDir2.isValid());
    QDir testDir2(tmpDir2.path());

    ChromeCookieStore store;
    QSignalSpy dbPathChanged(&store, SIGNAL(dbPathChanged()));

    QString path = testDir.filePath("cookies.db");
    store.setProperty("dbPath", path);
    QCOMPARE(dbPathChanged.count(), 1);
    QCOMPARE(store.property("dbPath").toString(), path);
    dbPathChanged.clear();

    QString path2 = testDir2.filePath("cookies.db");
    store.setProperty("dbPath", "file://" + path2);
    QCOMPARE(dbPathChanged.count(), 1);
    QCOMPARE(store.property("dbPath").toString(), path2);

    QVERIFY(store.property("cookies").value<Cookies>().isEmpty());
}

void CookieStoreTest::testWebkitProperties()
{
    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QDir testDir(tmpDir.path());

    WebkitCookieStore store;
    QSignalSpy dbPathChanged(&store, SIGNAL(dbPathChanged()));

    QString path = testDir.filePath("cookies.db");
    store.setProperty("dbPath", path);
    QCOMPARE(dbPathChanged.count(), 1);
    QCOMPARE(store.property("dbPath").toString(), path);

    QVERIFY(store.property("cookies").value<Cookies>().isEmpty());
}

void CookieStoreTest::setupCookieData()
{
    QTest::addColumn<Cookies>("cookies");

    Cookies cookies;

    cookies << "LSID=DQAAAKEaem_vYg; Domain=docs.foo.com; Path=/accounts; "
        "Expires=Wed, 13 Jan 2021 22:23:01 GMT; Secure; HttpOnly";
    QTest::newRow("Single cookie") << cookies;

    cookies.clear();
    cookies << "LSID=DQAAAKEaem_vYg; Domain=docs.foo.com; Path=/accounts; "
        "Expires=Wed, 13 Jan 2021 22:23:01 GMT; Secure; HttpOnly";
    cookies << "HSID=AYQEVnDKrdst; Domain=.foo.com; Path=/; "
        "Expires=Wed, 13 Jan 2021 22:23:01 GMT; HttpOnly";
    cookies << "SSID=Ap4PGTEq; Domain=foo.com; Path=/; "
        "Expires=Wed, 13 Jan 2021 22:23:01 GMT; Secure";
    cookies << "made_write_conn=1295214458; Path=/; Domain=.example.com";
    QTest::newRow("Few cookies") << cookies;
}

void CookieStoreTest::testChromeReadWrite()
{
    QFETCH(Cookies, cookies);

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QDir testDir(tmpDir.path());

    ChromeCookieStore store;
    QSignalSpy cookiesChanged(&store, SIGNAL(cookiesChanged()));
    store.setDbPath(testDir.filePath("cookies.db"));

    QCOMPARE(cookiesChanged.count(), 0);
    store.setProperty("cookies", QVariant::fromValue(cookies));
    QCOMPARE(cookiesChanged.count(), 1);
    Cookies readCookies = store.property("cookies").value<Cookies>();
    QCOMPARE(parseCookies(readCookies), parseCookies(cookies));
}

void CookieStoreTest::testWebkitReadWrite()
{
    QFETCH(Cookies, cookies);

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QDir testDir(tmpDir.path());

    WebkitCookieStore store;
    QSignalSpy cookiesChanged(&store, SIGNAL(cookiesChanged()));
    store.setDbPath(testDir.filePath("cookies.db"));

    QCOMPARE(cookiesChanged.count(), 0);
    store.setProperty("cookies", QVariant::fromValue(cookies));
    QCOMPARE(cookiesChanged.count(), 1);
    Cookies readCookies = store.property("cookies").value<Cookies>();
    QCOMPARE(parseCookies(readCookies), parseCookies(cookies));
}

void CookieStoreTest::testMoving()
{
    QFETCH(Cookies, cookies);

    QTemporaryDir tmpDir;
    QVERIFY(tmpDir.isValid());
    QDir testDir(tmpDir.path());

    WebkitCookieStore webkitStore;
    webkitStore.setDbPath(testDir.filePath("webkit.db"));
    webkitStore.setProperty("cookies", QVariant::fromValue(cookies));

    ChromeCookieStore chromeStore;
    chromeStore.setDbPath(testDir.filePath("chrome.db"));

    QSignalSpy moved(&chromeStore, SIGNAL(moved(bool)));
    chromeStore.moveFrom(&webkitStore);

    QCOMPARE(moved.count(), 1);
    QCOMPARE(moved.at(0).at(0).toBool(), true);

    Cookies movedCookies = chromeStore.property("cookies").value<Cookies>();
    QCOMPARE(parseCookies(movedCookies), parseCookies(cookies));
}

QTEST_MAIN(CookieStoreTest)
#include "tst_CookieStore.moc"