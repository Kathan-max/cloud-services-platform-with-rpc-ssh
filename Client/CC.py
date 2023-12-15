import xmlrpc.client

class CC:
    def __init__(self, server_url, t_api_key, api_key, username, video_ids):
        self.server_url = server_url
        self.t_api_key = t_api_key
        self.api_key = api_key
        self.username = username
        self.video_ids = video_ids

    def send_credentials(self):
        with xmlrpc.client.ServerProxy(self.server_url) as proxy:
            try:
                result = proxy.receive_youtube_credentials(self.t_api_key, self.api_key, self.username, self.video_ids)
                return result
            except Exception as e:
                print("An error occurred:", e)
                return None
            
    def do_analysis(self,method_name):
        with xmlrpc.client.ServerProxy(self.server_url) as proxy:
            
            try:
                result = proxy.call_method(self.username,self.api_key,method_name)
                return result
            except Exception as e:
                print("An error occurred:", e)
                return None

# Example usage:
if __name__ == "__main__":
    server_url = "http://10.20.24.87:8000/RPC2"
    t_api_key = "your_transitory_api_key_here"
    api_key = "your_permanent_api_key_here"
    username = "your_username"
    video_ids = ["video_id1", "video_id2", "video_id3"]

    cc_object = CC(server_url, t_api_key, api_key, username, video_ids)
    result = cc_object.send_credentials()
    print("Result:", result)
    
    
    res2 = cc_object.do_analysis("generate_summ")
    print(res2)
