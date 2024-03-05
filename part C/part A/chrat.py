import matplotlib.pyplot as plt

# Reno - Reno
reno_reno_speed = [7377.316432, 6894.314423, 6688.876205, 5620.420842]
reno_reno_time = [0.000278, 0.000305, 0.000328, 0.000367]

# Cubic - Reno
cubic_reno_speed = [5734.642591, 7170.122927, 5661.839648, 4927.775426]
cubic_reno_time = [0.000409, 0.000301, 0.000365, 0.000498]

# Reno - Cubic
reno_cubic_speed = [6523.625503, 6598.457185, 6749.017511, 4539.422292]
reno_cubic_time = [0.000317, 0.000322, 0.000314, 0.000619]

# Cubic - Cubic
cubic_cubic_speed = [6100.791909, 5857.570153, 6629.715432, 5744.640449]
cubic_cubic_time = [0.000335, 0.000385, 0.000302, 0.000351]

# Labels for the x-axis (scenarios)
labels = ['0%', '2%', '5%', '10%']

# Plotting the bar chart for Average Speed
x = range(len(labels))
width = 0.2

fig, ax = plt.subplots(2, 1, figsize=(10, 8))

# Plotting for Average Speed
ax[0].bar(x, reno_reno_speed, width, label='Reno to Reno')
ax[0].bar([i + width for i in x], reno_cubic_speed, width, label='Reno to Cubic')
ax[0].bar([i + 2*width for i in x], cubic_reno_speed, width, label='Cubic to Reno')
ax[0].bar([i + 3*width for i in x], cubic_cubic_speed, width, label='Cubic to Cubic')

ax[0].set_xlabel('Scenario')
ax[0].set_ylabel('Average Speed (MB/S)')
ax[0].set_title('Comparison of Average Speed between TCP Congestion Control Algorithms')
ax[0].set_xticks([i + 1.5*width for i in x])
ax[0].set_xticklabels(labels)
ax[0].legend()

# Plotting for Average Time
ax[1].bar(x, reno_reno_time, width, label='Reno to Reno')
ax[1].bar([i + width for i in x], reno_cubic_time, width, label='Reno to Cubic')
ax[1].bar([i + 2*width for i in x], cubic_reno_time, width, label='Cubic to Reno')
ax[1].bar([i + 3*width for i in x], cubic_cubic_time, width, label='Cubic to Cubic')

ax[1].set_xlabel('Scenario')
ax[1].set_ylabel('Average Time (S)')
ax[1].set_title('Comparison of Average Time between TCP Congestion Control Algorithms')
ax[1].set_xticks([i + 1.5*width for i in x])
ax[1].set_xticklabels(labels)
ax[1].legend()

plt.tight_layout()
plt.show()
