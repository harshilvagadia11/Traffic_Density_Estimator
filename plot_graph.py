import pandas as pd
import plotly.express as px

df = pd.read_csv('out.csv')

fig = px.line(df, x = 'Frame', y = 'Queue', title='queue')
fig.show()