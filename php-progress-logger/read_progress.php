<!doctype html>
<html>

    <head>
    <script src="https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js"></script>
     <script>


        $(document).ready(function(){
        var hostname = window.location.hostname;
        var url_string = window.location.href
        var url = new URL(url_string);

        var c = url.searchParams.get("upload_key");
        console.log(c);

            (function worker() {
              $.get( "http://" + hostname + '/progress_logs/request_log.php?upload_key=' + c, function(data) {
                // Now that we've completed the request schedule the next one.
                data = data + "\n" + "Last update time: " + new Date().toLocaleString() + " for key: " + c;
                $('.result').html(data);
                setTimeout(worker, 1000);
              });
            })();

        });
    </script>
    </head>
    <body>

        <div class="result">

        </div>


    </body>
</html>
