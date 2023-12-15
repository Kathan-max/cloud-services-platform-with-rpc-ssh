from CC import CC


server_url = "http://10.20.24.87:8000/RPC2"
t_api_key = "AIzaSyAPWgzIObW78c-Wmu1O1Rbu6pg8ZKYCnwE"
username = "kathan_me"
video_ids = ["9RJWPqnmllM","ErMSHiQRnc8","XxOh12Uhg08","TLKxdTmk-zc","yta_B6tq2VQ","GfO-3Oir-qM","4JOkwcA_quE","gIFSSmCkFEw","Qrjd2PcDyvE","e1cf58VWzt8","GMIawSAygO4","9iGM346vRM","_inKs4eeHiI"]


api_key = "67890efgh"        


cc_object = CC(server_url, t_api_key, api_key, username, video_ids)
res2 = cc_object.do_analysis("generate_summ")
print(res2)