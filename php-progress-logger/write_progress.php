<?php




    $current_record_log = $_POST['current_record_log'];
    $upload_key = $_POST['upload_key'];
    
    file_put_contents("upload_key_" . $upload_key . ".txt",htmlspecialchars($current_record_log));
   
    echo "wrote to file";
    

?>