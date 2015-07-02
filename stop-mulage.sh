echo "stopping asr services...."

kill -9 `pidof asrservice`

echo "stopping imm services...."

kill -9 `pidof imservice`

echo "stopping qa services....."

kill -9 `pidof java`
