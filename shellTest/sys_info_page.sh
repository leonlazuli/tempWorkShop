#!/bin/bash

#Leon's shell, output a system information

TITLE="System Infomation Report For $HOSTNAME"
CURRENT_TIME=$(date +"%x %r %Z")
TIME_STAMP="Generated $CURRENT_TIME, by $USER"


report_uptime () {
    cat <<- _EOF_
<H2>System Uptime</H2>
<PRE>$(uptime)</PRE>
_EOF_

}

report_home_space() {
    if [[ $(id -u) -eq 0  ]]; then
        cat <<- EOF
<H2> Home Space Utilization (All Users)</H2>
<PRE>$(du -sh /home/*)</PRE>
EOF
    else
        cat <<- EOF
        <H2>Home Space Utilization ($USER)</H2>
        <PRE>$(du -sh $HOME)</PRE>
EOF
    fi
    return

}

echo $(report_uptime);
report_uptime

cat <<- _EOF_
<HTML>
     <HEAD>
         <TITLE>$TITLE</TITLE>
     </HEAD>
     <BODY>
                <H1>$TITLE</H1>
                <p>$TIME_STAMP</p>
                $(report_uptime)
                $(report_home_space)
     </BODY>
</HTML>
_EOF_

