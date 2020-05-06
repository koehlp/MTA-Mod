<?php


        $upload_key = $_GET['upload_key'];
        $recordLog = file_get_contents("upload_key_" . $upload_key . ".txt");

        $your_array = explode("\n", $recordLog);

        foreach ($your_array as &$value) {
            echo htmlspecialchars($value) . "<br>";
        }


?>