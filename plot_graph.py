import pandas as pd
import plotly.express as px
import sys

df = pd.read_csv(sys.argv[1])

fig = px.line(df, x = 'Frame', y = ['Queue', 'Moving'], title='Graph')
fig.show()