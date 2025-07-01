# -*- coding: utf-8 -*-
"""
Created on Tue Jul  1 23:35:13 2025

@author: 10534
"""
# -*- coding: utf-8 -*-
"""
Created on Tue Jul  1 23:35:13 2025

@author: 10534
"""

import tkinter as tk
from tkinter import ttk, filedialog, messagebox, scrolledtext
import socket
import threading
import os
import struct


class TCPServerUI:
    def __init__(self, root):
        self.root = root
        self.root.title("TCP文件传输服务器")
        self.root.geometry("600x500")
        
        self.server_socket = None
        self.is_running = False
        self.selected_file = None
        
        self.create_widgets()
        
    def create_widgets(self):
        # 服务器配置框架
        config_frame = ttk.LabelFrame(self.root, text="服务器配置", padding="10")
        config_frame.pack(fill="x", padx=10, pady=5)
        
        # IP地址设置
        ttk.Label(config_frame, text="IP地址:").grid(row=0, column=0, sticky="w", padx=5)
        self.ip_var = tk.StringVar(value="192.168.1.5")
        self.ip_entry = ttk.Entry(config_frame, textvariable=self.ip_var, width=15)
        self.ip_entry.grid(row=0, column=1, padx=5)
        
        # 端口设置
        ttk.Label(config_frame, text="端口:").grid(row=0, column=2, sticky="w", padx=5)
        self.port_var = tk.StringVar(value="8087")
        self.port_entry = ttk.Entry(config_frame, textvariable=self.port_var, width=10)
        self.port_entry.grid(row=0, column=3, padx=5)
        
        # 启动/停止按钮
        self.start_btn = ttk.Button(config_frame, text="启动服务器", command=self.start_server)
        self.start_btn.grid(row=0, column=4, padx=10)
        
        self.stop_btn = ttk.Button(config_frame, text="停止服务器", command=self.stop_server, state="disabled")
        self.stop_btn.grid(row=0, column=5, padx=5)
        
        # 文件选择框架
        file_frame = ttk.LabelFrame(self.root, text="文件传输", padding="10")
        file_frame.pack(fill="x", padx=10, pady=5)
        
        # 文件路径显示
        ttk.Label(file_frame, text="选择文件:").grid(row=0, column=0, sticky="w", padx=5)
        self.file_var = tk.StringVar(value="未选择文件")
        self.file_label = ttk.Label(file_frame, textvariable=self.file_var, width=50, relief="sunken")
        self.file_label.grid(row=0, column=1, padx=5, sticky="ew")
        
        # 选择文件按钮
        self.select_btn = ttk.Button(file_frame, text="选择文件", command=self.select_file)
        self.select_btn.grid(row=0, column=2, padx=5)
        
        # 发送文件按钮
        self.send_btn = ttk.Button(file_frame, text="发送文件", command=self.send_file, state="disabled")
        self.send_btn.grid(row=0, column=3, padx=5)
        
        file_frame.columnconfigure(1, weight=1)
        
        # 状态显示框架
        status_frame = ttk.LabelFrame(self.root, text="服务器状态", padding="10")
        status_frame.pack(fill="both", expand=True, padx=10, pady=5)
        
        # 日志显示
        self.log_text = scrolledtext.ScrolledText(status_frame, height=15, width=70)
        self.log_text.pack(fill="both", expand=True)
        
        # 客户端列表
        client_frame = ttk.Frame(status_frame)
        client_frame.pack(fill="x", pady=5)
        
        ttk.Label(client_frame, text="连接的客户端:").pack(side="left")
        self.client_var = tk.StringVar(value="无客户端连接")
        ttk.Label(client_frame, textvariable=self.client_var).pack(side="left", padx=10)
        
        self.clients = []
        
    def log_message(self, message):
        """在日志区域显示消息"""
        self.log_text.insert(tk.END, f"{message}\n")
        self.log_text.see(tk.END)
        
    def start_server(self):
        """启动TCP服务器"""
        try:
            ip = self.ip_var.get()
            port = int(self.port_var.get())
            
            self.server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            self.server_socket.bind((ip, port))
            self.server_socket.listen(5)
            
            self.is_running = True
            self.start_btn.config(state="disabled")
            self.stop_btn.config(state="normal")
            self.ip_entry.config(state="disabled")
            self.port_entry.config(state="disabled")
            
            self.log_message(f"服务器启动成功，监听 {ip}:{port}")
            
            # 启动接受连接的线程
            threading.Thread(target=self.accept_connections, daemon=True).start()
            
        except Exception as e:
            messagebox.showerror("错误", f"启动服务器失败: {str(e)}")
            self.log_message(f"启动服务器失败: {str(e)}")
            
    def stop_server(self):
        """停止TCP服务器"""
        self.is_running = False
        
        # 关闭所有客户端连接
        for client in self.clients:
            try:
                client.close()
            except:
                pass
        self.clients.clear()
        
        # 关闭服务器socket
        if self.server_socket:
            try:
                self.server_socket.close()
            except:
                pass
            
        self.start_btn.config(state="normal")
        self.stop_btn.config(state="disabled")
        self.send_btn.config(state="disabled")
        self.ip_entry.config(state="normal")
        self.port_entry.config(state="normal")
        
        self.client_var.set("无客户端连接")
        self.log_message("服务器已停止")
        
    def accept_connections(self):
        """接受客户端连接"""
        while self.is_running:
            try:
                client_socket, address = self.server_socket.accept()
                self.clients.append(client_socket)
                self.log_message(f"客户端连接: {address[0]}:{address[1]}")
                self.client_var.set(f"已连接客户端: {len(self.clients)} 个")
                
                # 如果有文件选择，启用发送按钮
                if self.selected_file:
                    self.send_btn.config(state="normal")
                    
                # 启动处理客户端的线程
                threading.Thread(target=self.handle_client, args=(client_socket, address), daemon=True).start()
                
            except Exception as e:
                if self.is_running:
                    self.log_message(f"接受连接错误: {str(e)}")
                break
                
    def handle_client(self, client_socket, address):
        """处理客户端连接"""
        try:
            while self.is_running:
                # 保持连接，等待命令
                data = client_socket.recv(1024)
                if not data:
                    break
                    
                command = data.decode('utf-8').strip()
                if command == "REQUEST_FILE":
                    self.log_message(f"客户端 {address[0]}:{address[1]} 请求文件")
                    
        except Exception as e:
            self.log_message(f"处理客户端 {address[0]}:{address[1]} 错误: {str(e)}")
        finally:
            try:
                client_socket.close()
                if client_socket in self.clients:
                    self.clients.remove(client_socket)
                self.client_var.set(f"已连接客户端: {len(self.clients)} 个")
                self.log_message(f"客户端 {address[0]}:{address[1]} 断开连接")
                
                # 如果没有客户端连接，禁用发送按钮
                if not self.clients:
                    self.send_btn.config(state="disabled")
                    
            except:
                pass
                
    def select_file(self):
        """选择要传输的文件"""
        file_path = filedialog.askopenfilename(
            title="选择要传输的文件",
            filetypes=[("所有文件", "*.*")]
        )
        
        if file_path:
            self.selected_file = file_path
            self.file_var.set(os.path.basename(file_path))
            self.log_message(f"已选择文件: {file_path}")
            
            # 如果有客户端连接，启用发送按钮
            if self.clients and self.is_running:
                self.send_btn.config(state="normal")
                
    def send_file(self):
        """向所有连接的客户端发送文件"""
        if not self.selected_file or not os.path.exists(self.selected_file):
            messagebox.showerror("错误", "请先选择有效的文件")
            return
            
        if not self.clients:
            messagebox.showerror("错误", "没有客户端连接")
            return
            
        try:
            file_name = os.path.basename(self.selected_file)
            file_size = os.path.getsize(self.selected_file)
            
            self.log_message(f"开始发送文件: {file_name} ({file_size} 字节)")
            
            # 向所有客户端发送文件
            for client in self.clients[:]:  # 创建副本以避免修改列表时的问题
                try:
                    # 发送文件信息
                    #file_info = f"{file_name}|{file_size}".encode('utf-8')
                    #client.send(struct.pack('I', len(file_info)))
                    #client.send(file_info)
                    client.send("UPDATE!".encode('utf-8'))
                    
                    client.send("dummy!".encode('utf-8'))
                    
                    client.send("OVER!".encode('utf-8'))
                    # 发送文件内容
                    """
                    with open(self.selected_file, 'rb') as f:
                        sent_bytes = 0
                        while sent_bytes < file_size:
                            chunk = f.read(4096)
                            if not chunk:
                                break
                            client.send(chunk)
                            sent_bytes += len(chunk)
                       
                    self.log_message(f"文件发送完成: {sent_bytes} 字节")
                    """
                except Exception as e:
                    self.log_message(f"发送文件到客户端失败: {str(e)}")
                    if client in self.clients:
                        self.clients.remove(client)
                        
        except Exception as e:
            messagebox.showerror("错误", f"发送文件失败: {str(e)}")
            self.log_message(f"发送文件失败: {str(e)}")


def main():
    root = tk.Tk()
    app = TCPServerUI(root)
    root.mainloop()


if __name__ == "__main__":
    main()