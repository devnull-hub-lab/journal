**Keep a daily log of what you learned, about your life, ideas, questions, topics you
and you are interested. The practice of dump brain on a plain text file.**

## About
A daemon that reads .journal user file and return data to client.
A tool that includes your journal in (obviously) .journal

## Install

*Dependencies to build journald: **sqlite3***

*Instructions to build and install daemon and tool:*
```
make
make install
```

**Run Daemon:** /usr/local/sbin/journald
**Run Tool:** /usr/local/bin/journaladd

## Install Daemon on Systemd Init System (journal.service)
Create file on /etc/systemd/system/journal.service
```sh
cat << EOF > /etc/systemd/system/journal.service
[Unit]
Description=Simple Daily Journaling
After=network.target

[Service]
Type=simple
User=journal
ExecStart=/usr/local/sbin/journald
ExecReload=/bin/kill -HUP $MAINPID
Restart=on-failure

[Install]
WantedBy=multi-user.target
EOF
```

### Enable service and start!
```sh
systemctl enable journal
systemctl start journal
```
