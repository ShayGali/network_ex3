import matplotlib.pyplot as plt

# TCP
tcp_speed = [7377.316432, 6894.314423, 6688.876205, 5620.420842]
tcp_time = [0.000278, 0.000305, 0.000328, 0.000367]

#RUDP
rudp_speed = []
rudp_time = []
# Labels for the x-axis (scenarios)
labels = ['0%', '2%', '5%', '10%']

# Plotting the bar chart for Average Speed
x = range(len(labels))
width = 0.2

fig, ax = plt.subplots(2, 1, figsize=(10, 8))

# Plotting for Average Speed
ax[0].bar(x, tcp_speed, width, label='TCP')
ax[0].bar([i + width for i in x], rudp_time, width, label='RUDP')

ax[0].set_xlabel('Scenario')
ax[0].set_ylabel('Average Speed (MB/S)')
ax[0].set_title('Comparison of Average Speed between TCP and RUDP')
ax[0].set_xticks([i + 1.5*width for i in x])
ax[0].set_xticklabels(labels)
ax[0].legend()

# Plotting for Average Time
ax[1].bar(x, tcp_time, width, label='TCP')
ax[1].bar([i + width for i in x], rudp_time, width, label='RUDP')

ax[1].set_xlabel('Scenario')
ax[1].set_ylabel('Average Time (S)')
ax[1].set_title('Comparison of Average Time between TCP and RUDP')
ax[1].set_xticks([i + 1.5*width for i in x])
ax[1].set_xticklabels(labels)
ax[1].legend()

plt.tight_layout()
plt.show()
