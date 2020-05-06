# importing the requests library
import requests
import time

# api-endpoint
URL = "http://gtav.atspace.eu/write_progress.php"

content = ""


secondCount = 0

while True:
    try:

        time.sleep(1)
        secondCount += 1
        print("running " + str(secondCount) + "s")
        f = open("D:\GTA5\MTMCT\log_ped_spawn_progress.txt", "r")
        readContent = f.read()
        f.close()
        if content != readContent:
            content = readContent
            lines = content.split("\n")


            # defining a params dict for the parameters to be sent to the API
            PARAMS = {'current_record_log': "\n".join(lines[-150:])}

            # sending get request and saving the response as response object
            r = requests.get(url=URL, params=PARAMS)
            print("content: " + str(content))
            print("uploaded with status code: " + str(r.status_code))

    except BaseException as e:
        print(e)



