
# CÓDIGO DA Interface Home Máquina (IHM).
# python3.6
# https://www.emqx.com/en/blog/how-to-use-mqtt-in-python
import time
import random
import matplotlib.pyplot as plt
import matplotlib.animation as animation

from paho.mqtt import client as mqtt_client

fig = plt.figure()
ax1 = fig.add_subplot(1,1,1)
def animate(i):
    ax1.clear()
    # ax1.plot(xar, analog, marker = 'o', mfc = 'r', mec = 'r')
    ax1.plot(xar, analog, marker = 'o', color='dodgerblue', mfc = 'dodgerblue', mec = 'dodgerblue')
    ax1.plot(xar, s1dig, marker = 'o', color='forestgreen', mfc = 'forestgreen', mec = 'forestgreen')
    ax1.plot(xar, s2dig, marker = 'o', color='darkviolet', mfc = 'darkviolet', mec = 'darkviolet')
    plt.grid()
ani = animation.FuncAnimation(fig, animate, interval=1000)

xar = ['10ª', '09ª', '08ª', '07ª', '06ª', '05ª', '04ª', '03ª', '02ª', '01ª']

analog = [-1, -1, -1, -1, -1, -1, -1, -1, -1, -1]
s1dig = [-2, -2, -2, -2, -2, -2, -2, -2, -2, -2]
s2dig = [-3, -3, -3, -3, -3, -3, -3, -3, -3, -3]
indice = 1
# broker = 'broker.emqx.io'
broker = '10.0.0.101'
port = 1883
# topic = "TSRVINI111"
tGraph = "GRAPHT"
tAnal = "an4log"
tSd1g = "d1g"
tSd2g = "d2g"
client_id = f'subscribe-{random.randint(0, 100)}'
# username = 'jvsvl'
# password = 'jvsvl'
username = 'aluno'
password = '@luno*123'

def connect_mqtt() -> mqtt_client:
    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("Connected to MQTT Broker!")
        else:
            print("Failed to connect, return code %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)
    client.on_connect = on_connect
    client.connect(broker, port)
    return client

def subscribe(client: mqtt_client):
    def on_message(client, userdata, msg):
        global indice
        global fig
        global ax1
        global ani
        global analog
        global s1dig
        global s2dig
        msgTopic = msg.topic
        # print(msgTopic)
        if msgTopic == tAnal:
            # print("A    N   A   L")
            part = int(msg.payload.decode())
            if(part > 1023):
                print("READING ERROR")
            else:
                analog.pop(0)
                # analog.insert(9, int(msg.payload.decode()))
                analog.insert(9, part)
        if msgTopic == tSd1g:
            s1dig.pop(0)
            s1dig.insert(9, int(msg.payload.decode()))
        if msgTopic == tSd2g:
            s2dig.pop(0)
            s2dig.insert(9, int(msg.payload.decode()))
        if tGraph == msg.payload.decode():
            print("FECHAR GRÁFICO")
            plt.close('all')
            # time.sleep(1)
            analog = [-1, -1, -1, -1, -1, -1, -1, -1, -1, -1]
            s1dig = [-2, -2, -2, -2, -2, -2, -2, -2, -2, -2]
            s2dig = [-3, -3, -3, -3, -3, -3, -3, -3, -3, -3]
            fig = plt.figure()
            ax1 = fig.add_subplot(1,1,1)
            ani = animation.FuncAnimation(fig, animate, interval=100)
        print(f"Received '{msg.payload.decode()}' from '{msg.topic}' topic")
        plt.ion()
        plt.show()
        plt.draw()
        plt.pause(0.100)

    # client.subscribe(topic)
    client.subscribe(tAnal)
    client.subscribe(tSd1g)
    client.subscribe(tSd2g)
    client.subscribe(tGraph)
    client.on_message = on_message

def run():
    client = connect_mqtt()
    subscribe(client)
    client.loop_forever()

if __name__ == '__main__':
    run()

