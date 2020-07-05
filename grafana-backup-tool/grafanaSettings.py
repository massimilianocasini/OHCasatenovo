import os

grafana_url = os.getenv('GRAFANA_URL', 'http://localhost:3000')
token = os.getenv('GRAFANA_TOKEN', 'eyJrIjoiV3dIQWxsUjRPWUlmdURBVlRaMUZuNXZ4NkJaMXBxd3AiLCJuIjoiQmFja3VwX1Jlc3RvcmUiLCJpZCI6MX0=')
http_get_headers = {'Authorization': 'Bearer ' + token}
http_post_headers = {'Authorization': 'Bearer ' + token, 'Content-Type': 'application/json'}

