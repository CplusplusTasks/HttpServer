<?php

switch ($_REQUEST['action']) {
    case 'sample1':
        echo 'Пример 1 - передача завершилась успешно';
        break;
    case 'sample2':
        echo 'Пример 2 - передача завершилась успешно. Параметры: name = ' . $_POST['name'] . ', nickname= ' . $_POST['nickname'];
        break;
    case 'sample3':
        echo "$('.results').html('Пример 3 - Выполнение JavaScript');";
        break;
    case 'sample4':
        header ('Content-Type: application/xml; charset=UTF-8');

        echo <<<XML
<?xml version='1.0' standalone='yes'?>
<items>
<item>Пункт 1</item>
<item>Пункт 2</item>
<item>Пункт 3</item>
<item>Пункт 4</item>
<item>Пункт 5</item>
</items>
XML;
        break;
    case 'sample5':
        $aRes = array('name' => 'Andrew', 'nickname' => 'Aramis');

        require_once('Services_JSON.php');
        $oJson = new Services_JSON();
        echo $oJson->encode($aRes);
        break;
}

?>