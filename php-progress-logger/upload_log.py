# importing the requests library
import time

import argparse


import urllib.parse
import urllib.request

import signal

exit_loop = False

def handler(signum, frame):
    global exit_loop

    exit_loop = True
    print("crtl c pressed")

def run_uploader(upload_url, upload_file_path, upload_key):
    global exit_loop
    signal.signal(signal.SIGINT, handler)

    # api-endpoint


    content = ""


    secondCount = 0

    while True:
        try:


            time.sleep(1)
            secondCount += 1
            print("running " + str(secondCount) + "s")
            f = open(upload_file_path, "r")
            readContent = f.read()
            f.close()
            if True or content != readContent:
                content = readContent
                lines = content.split("\n")

                # defining a params dict for the parameters to be sent to the API
                PARAMS = {'upload_key' : upload_key , 'current_record_log': "\n".join(lines[-150:])}

                data = urllib.parse.urlencode(PARAMS).encode("utf-8")
                req = urllib.request.Request(upload_url, data)
                response = urllib.request.urlopen(req,timeout=5)
                the_page = response.read().decode("utf-8")

                print("Last sent lines: \n" +  "\n".join(lines[-3:]))
                print(the_page)





        except BaseException as e:
            print(e)

        if exit_loop:
            return


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='Videos to images')
    parser.add_argument('--file', type=str, help='Input log file to upload.',required=True)
    parser.add_argument('--key', type=str, help='Key to access the log file online.',required=True)
    parser.add_argument('--url', type=str, help='URL to which the log file should be uploaded.', default="http://example.com/progress_logs/write_progress.php")

    args = parser.parse_args()
    print(args)

    run_uploader(upload_url=args.url,upload_file_path=args.file,upload_key=args.key)

    #python upload_log.py --file log.txt --key cascade_abd_test



