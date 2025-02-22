#!/usr/bin/env -S uv --quiet run --script
# /// script
# requires-python = ">=3.13"
# dependencies = [
#     "plotly",
#     "numpy",
# ]
# ///
import sys

import plotly.graph_objects as go
import numpy as np

# sox 1.wav -r 48000 -c 1 -b 16 -e float output.raw
SAMPLE_RATE = 48000
# Load output.raw which has 1 channel of floats
with open("output.raw", "rb") as f:
    data = np.frombuffer(f.read(), dtype=np.float32)

with open("follower.raw", "rb") as f:
    follower = np.frombuffer(f.read(), dtype=np.float32)

x = np.linspace(0, len(data) / SAMPLE_RATE, len(data))

# Create a figure
fig = go.Figure()

# Add a trace
fig.add_trace(go.Scatter(x=x, y=data, mode="lines", name="Signal"))

# Add the follower signal in red
fig.add_trace(
    go.Scatter(x=x, y=follower, mode="lines", line=dict(color="red"), name="Envelope")
)

# Label x axis Time (s)
fig.update_xaxes(title_text="Time (s)")
fig.update_yaxes(title_text="Amplitude")


# Show the figure
fig.show()
