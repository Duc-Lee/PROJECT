from flask import Flask, render_template, request, jsonify, make_response
import subprocess
import os
import json
import time
import logging
from dijkstra import find_shortest_path

# Cấu hình logging
logging.basicConfig(level=logging.DEBUG)
logger = logging.getLogger(__name__)

app = Flask(__name__)

# Danh sách các tòa nhà hợp lệ
VALID_BUILDINGS = [
    "Toa C1", "Toa C2", "Toa C3", "Toa C4", "Toa C5", "Toa C6", "Toa C7", "Toa C8", "Toa C9", "Toa C10",
    "Toa D3", "Toa D4", "Toa D5", "Toa D6", "Toa D7", "Toa D8", "Toa D9",
    "Thu Vien", "Nha TC"
]

# Biến toàn cục để lưu kết quả mới nhất
latest_result = None
RESULT_FILE = "path_result.txt"

def add_cors_headers(response):
    if isinstance(response, tuple):
        response_obj, status_code = response
        response_obj = make_response(response_obj)
        response_obj.status_code = status_code
    else:
        response_obj = make_response(response)
    
    response_obj.headers['Access-Control-Allow-Origin'] = '*'
    response_obj.headers['Access-Control-Allow-Headers'] = 'Content-Type'
    response_obj.headers['Access-Control-Allow-Methods'] = 'GET, POST, OPTIONS'
    return response_obj

def read_result_file():
    try:
        if not os.path.exists(RESULT_FILE):
            logger.debug(f"Result file {RESULT_FILE} does not exist")
            return None
        
        with open(RESULT_FILE, 'r') as f:
            lines = f.readlines()
            if len(lines) < 2:
                logger.debug(f"Result file has insufficient lines: {len(lines)}")
                return None
            
            path = lines[0].strip().split(' -> ')
            try:
                distance = int(lines[1].strip())
            except ValueError:
                logger.debug(f"Invalid distance value in result file: {lines[1].strip()}")
                return None
            
            return {
                'path': path,
                'distance': distance
            }
    except Exception as e:
        logger.error(f"Error reading result file: {str(e)}")
        return None

@app.route('/')
def index():
    return render_template('index.html')

@app.route('/get_result')
def get_result():
    global latest_result
    logger.debug("GET /get_result called")
    result = read_result_file()
    if result:
        latest_result = result
    if latest_result is None:
        response = jsonify({'error': 'Chưa có kết quả'}), 404
        return add_cors_headers(response)
    response = jsonify(latest_result)
    return add_cors_headers(response)

@app.route('/find_path', methods=['POST', 'OPTIONS'])
def find_path():
    if request.method == 'OPTIONS':
        response = make_response()
        return add_cors_headers(response)
        
    try:
        logger.debug("POST /find_path called")
        data = request.get_json()
        if not data:
            logger.debug("No JSON data received")
            response = jsonify({'error': 'Không có dữ liệu được gửi đến'}), 400
            return add_cors_headers(response)

        source = data.get('source')
        destination = data.get('destination')
        logger.debug(f"Source: {source}, Destination: {destination}")

        # Kiểm tra đầu vào
        if not source or not destination:
            logger.debug("Missing source or destination")
            response = jsonify({'error': 'Thiếu thông tin điểm đi hoặc điểm đến'}), 400
            return add_cors_headers(response)
        
        if source not in VALID_BUILDINGS:
            logger.debug(f"Invalid source: {source}")
            response = jsonify({'error': f'Điểm đi "{source}" không hợp lệ'}), 400
            return add_cors_headers(response)
            
        if destination not in VALID_BUILDINGS:
            logger.debug(f"Invalid destination: {destination}")
            response = jsonify({'error': f'Điểm đến "{destination}" không hợp lệ'}), 400
            return add_cors_headers(response)
            
        if source == destination:
            logger.debug("Source and destination are the same")
            response = jsonify({'error': 'Điểm đi và điểm đến không được trùng nhau'}), 400
            return add_cors_headers(response)

        # Xóa file kết quả cũ nếu tồn tại
        if os.path.exists(RESULT_FILE):
            os.remove(RESULT_FILE)
            logger.debug(f"Removed old result file: {RESULT_FILE}")

        # Thực thi chương trình C
        try:
            logger.debug("Running C program")
            input_data = f"{source}\n{destination}\n"
            result = subprocess.run(['./project'], 
                                 input=input_data,
                                 capture_output=True,
                                 text=True,
                                 check=True)
            logger.debug(f"C program output: {result.stdout}")
            
            # Đợi file kết quả được tạo
            max_wait = 5  # Số giây tối đa đợi
            wait_time = 0
            while not os.path.exists(RESULT_FILE) and wait_time < max_wait:
                time.sleep(0.1)
                wait_time += 0.1
                logger.debug(f"Waiting for result file... {wait_time}s")

            if not os.path.exists(RESULT_FILE):
                logger.error("Result file not created after timeout")
                response = jsonify({'error': 'Không nhận được kết quả từ chương trình tìm đường'}), 500
                return add_cors_headers(response)

            # Đọc kết quả từ file
            result_data = read_result_file()
            if not result_data:
                logger.error("Invalid result data")
                response = jsonify({'error': 'Kết quả không hợp lệ'}), 500
                return add_cors_headers(response)

            logger.debug(f"Successfully got result: {result_data}")
            response = jsonify(result_data)
            return add_cors_headers(response)

        except subprocess.CalledProcessError as e:
            error_msg = f"Lỗi khi chạy chương trình tìm đường: {e.stderr}"
            logger.error(error_msg)
            response = jsonify({'error': error_msg}), 500
            return add_cors_headers(response)
        except FileNotFoundError:
            logger.error("C program not found")
            response = jsonify({'error': 'Không tìm thấy chương trình tìm đường (project)'}), 500
            return add_cors_headers(response)

    except Exception as e:
        error_msg = f"Lỗi server không xác định: {str(e)}"
        logger.error(error_msg)
        response = jsonify({'error': error_msg}), 500
        return add_cors_headers(response)

if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0', port=5000)
