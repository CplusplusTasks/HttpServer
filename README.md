Tic-tac-toe
===

Сетевая игра крестики нолики. Запускается сервер, после к нему могут подключаться игроки через браузер. 

Для запуска сервера `./run.sh address port` (by default 127.0.0.1 7777), после сервер будет принимать соединения по адрессу `address` и порту `port`. 

`run.sh clean` - удаляет собранный проект

Подключение игроков осуществляется через браузер введя в адрессную строку `address:port`.

В игре можно создавать поле разного размера и ждать подключения к тебе второго игрока или же выбрать из списка игрока с кем ты хочешь играть. Все игроки играют независимо друг от друга.

http://sorokin.github.io/cpp-year2013/task-course-work.html

<html xmlns="http://www.w3.org/1999/xhtml">
  <head>
    <meta http-equiv="Content-Type" content="text/html; charset=utf-8"/>
  
  </head>
  <body>
    <h2>Курсовик за 2 семестр</h2>
    <p>В данном задании предлагается на выбор реализовать программу, использующую для работы с сетью <a href="https://en.wikipedia.org/wiki/Berkeley_sockets">BSD-сокеты</a> и
    реализующую один из популярных интернет протоколов. Примеры допустимых программ:</p>
    <ul>
      <li><a href="https://en.wikipedia.org/wiki/Hypertext_Transfer_Protocol">HTTP</a>-<a href="https://en.wikipedia.org/wiki/Proxy_server">proxy</a> сервер.</li>
      <li>Рекурсивный <a href="https://en.wikipedia.org/wiki/Wget">wget</a></li>
      <li>Простенькое <a href="https://en.wikipedia.org/wiki/Web_application">веб-приложение</a>:<br/>
        <ul>
          <li>Чат</li>
          <li><a href="https://en.wikipedia.org/wiki/News_aggregator">RSS-агрегатор</a></li>
          <li>Wiki</li>
        </ul>
      </li>
    </ul>
    <p>Допускается написание других программ, но это необходимо предварительно согласовать со мной.</p>
    <p>Набор требований/нетребований к реализации:</p>
    <ul>
      <li>Асинхронное выгребание событий от сокетов: epoll<sup id="cite_ref_1"><a href="#cite_note_1">[1]</a></sup> <sup id="cite_ref_2"><a href="#cite_note_2">[2]</a></sup>(Linux),
      I/O completion ports<sup id="cite_ref_3"><a href="#cite_note_3">[3]</a></sup> (Windows), kqueue<sup id="cite_ref_4"><a href="#cite_note_4">[4]</a></sup> (FreeBSD) </li>
      <li>Программа должна реагировать на посылаемые ей SIGINT, SIGTERM и <a href="http://en.wikipedia.org/wiki/Graceful_exit">gracefully</a> выходить.
      Именно не киляться, а выходить. Под словом выходить понимается выходить из функции main.</li>
      <li>Нельзя использовать библиотеки для работы с сокетами, кроме BSD-sockets.</li>
      <li>Нельзя использовать библиотеки, реализующие интернет протоколы.</li>
      <li>Для вещей не связанных с сетью, сторонние библиотеки использовать можно.</li>
    </ul>
    <p>Предполагается, что разрабатываемую программу условно можно разделить на 3 части:</p>
    <ol>
      <li id="part_1">Некоторая обертка над сокетами/epoll'ом, которая реализует TCP и ничего не знает про протокол вышележащего уровня.</li>
      <li id="part_2">Реализация соответствующего интернет протокола, например HTTP, использующая <a href="#part_1">1</a> и ничего не знающая про прикладную логику.</li>
      <li id="part_3">Прикладная логика, использующая <a href="#part_2">2</a>.</li>
    </ol>
<ul>
    <p>Утверждается, что если программу нельзя будет легко разделить на эти 3 части, то её код это <a href="https://en.wikipedia.org/wiki/Spaghetti_code">жуткая лапша</a>. Возможно для некоторых программ это не так,
    надо смотреть на конкретном примере. Но есть шанс, что это так. Поэтому вам предлагается сразу писать программу разделяя её на эти три части: две библиотеки
    и сама программа.</p>
    <p>Интерфейс части <a href="#part_1">1</a> может быть произвольным, но вы можете черпать вдохновение глядя на существующие библиотеки, например
    QTcpServer<sup id="cite_ref_5"><a href="#cite_note_5">[5]</a></sup>, QTcpSocket<sup id="cite_ref_6"><a href="#cite_note_6">[6]</a></sup>. Аналогично для
    части <a href="#part_2">2</a>: QNetworkAccessManager<sup id="cite_ref_7"><a href="#cite_note_7">[7]</a>.</p>

    <h3>Сроки</h3>
    
      <li>Работающую часть <a href="#part_1">1</a> необходимо показать 27 ноября.</li>
      <li>Работающую часть <a href="#part_2">2</a> необходимо показать 11 декабря.</li>
      <li>Работающую программу необходимо показать на зачетной неделе.</li>
    

    <h3>Ссылки</h3>
    <ol>
      <li id="cite_note_1"><b><a href="#cite_ref_1">^</a></b> <a href="http://linux.die.net/man/4/epoll">man 4 epoll</a></li>
      <li id="cite_note_2"><b><a href="#cite_ref_2">^</a></b> <a href="http://linux.die.net/man/7/epoll">man 7 epoll</a></li>
      <li id="cite_note_3"><b><a href="#cite_ref_3">^</a></b> <a href="http://msdn.microsoft.com/en-us/library/windows/desktop/aa365198(v=vs.85).aspx">I/O Completion Ports</a></li>
      <li id="cite_note_4"><b><a href="#cite_ref_4">^</a></b> <a href="http://www.freebsd.org/cgi/man.cgi?query=kqueue&sektion=2">man 2 kqueue</a></li>
      <li id="cite_note_5"><b><a href="#cite_ref_5">^</a></b> <a href="http://qt-project.org/doc/qt-5/qtcpserver.html">QTcpServer Class</a></li>
      <li id="cite_note_6"><b><a href="#cite_ref_6">^</a></b> <a href="http://qt-project.org/doc/qt-5/qtcpsocket.html">QTcpSocket Class</a></li>
      <li id="cite_note_7"><b><a href="#cite_ref_7">^</a></b> <a href="http://qt-project.org/doc/qt-5/qnetworkaccessmanager.html">QNetworkAccessManager Class</a></li>
    </ol>

    <p>
    
    </p></ul>
  </body>
</html>
