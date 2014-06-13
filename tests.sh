#!/bin/bash

function cleanup {
        echo "Killing zombies"
        ps aux|grep yacuws|grep Z| awk '{print $2}' | xargs kill -9

        echo "Killing yacuws"
        ps aux|grep './yacuws'| awk '{print $2}' | xargs kill -9

}

echo "-> Starting & daemonizing yacuws\n"
./yacuws > /tmp/yacuws-out&

echo -n "-> Testing a simple file retrieval for OK response: "

curl -v 'http://127.0.0.1:12345/LICENSE' 2>&1 >/dev/null | grep "HTTP/1.1 200 OK"> /dev/null;
if [[ $? -eq 0 ]]; then
        echo "OK"
else
        echo "FAIL"
        cleanup
        exit 1
fi

echo -n "-> Testing a simple file retrieval for file contents: "
curl -v 'http://127.0.0.1:12345/LICENSE' 2>&1 >/dev/null | grep "Content-Length: 1078"> /dev/null;
if [[ $? -eq 0 ]]; then
        echo "OK"
else
        echo "FAIL"
        cleanup
        exit 1
fi

echo -n "-> Testing a simple file retrieval with other method than GET: "
curl -X POST -d 'some=data&foo=bar' -v 'http://127.0.0.1:12345/LICENSE' 2>&1 >/dev/null | grep "HTTP/1.1 501 Not Implemented" > /dev/null;
if [[ $? -eq 0 ]]; then
        echo "OK"
else
        echo "FAIL"
        cleanup
        exit 1
fi


echo -n "-> Testing a simple file retrieval with invalid path: "
curl -v 'http://127.0.0.1:12345/LICENSE..' 2>&1 >/dev/null | grep "HTTP/1.1 400 Bad Request"> /dev/null;
if [[ $? -eq 0 ]]; then
        echo "OK"
else
        echo "FAIL"
        cleanup
        exit 1
fi

echo -n "-> Testing the directory listing: "
curl -v 'http://127.0.0.1:12345/' 2>/dev/null | grep "Dir listing"> /dev/null;
if [[ $? -eq 0 ]]; then
        echo "OK"
else
        echo "FAIL"
        cleanup
        exit 1
fi



cleanup
exit 0
