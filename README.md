Keep a daily log of what you learned, about your life, ideas, questions, topics you
and you are interested. The practice of dump brain on a plain text file.

## Install
A daemon that reads .journal user file and return data do sender

Dependencies to build journald: sqlite3

Instructions to build and install daemon:
make && make install

Run Daemon: /usr/local/sbin/journald

## On Systemd (journal.service)
Create file on /etc/systemd/system/journal.service

cat << EOF > /etc/systemd/system/journal.service
[Unit]
Description=Simple Daily Journaling
After=network.target

[Service]
Type=simple
User=journal
WorkingDirectory=/usr/local/sbin/
ExecStart=/usr/local/sbin/journald
ExecReload=/bin/kill -HUP $MAINPID
Restart=on-failure

[Install]
WantedBy=multi-user.target
EOF

### Enable service and start!
systemctl enable journal
systemctl start journal

