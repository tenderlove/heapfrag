require 'opengl'
require 'glu'
require 'glut'
require 'json'
require 'socket'

include Gl,Glu,Glut

HEIGHT = 200
SQ_PX  = 2

BitMap = Struct.new :width, :height, :young, :oldies

def make_bitmap heap
  heap = heap.find_all { |p| p.key? 'page' }.map do |page|
    page['heap']
  end

  max_page_size = heap.map(&:length).max
  heap.each { |page| (max_page_size - page.length).times { page << 0 } }

  columns = heap
  rows = columns.transpose

  youngs = []
  olds = []

  rows.each do |row|
    unless row.length % SQ_PX == 0
      row += [0] * (SQ_PX - (row.length % SQ_PX))
    end

    translated_row = row.each_slice(8 / SQ_PX).map { |slice|
      slice.inject(0) { |acc,i|
        acc <<= 2
        if i == 1
          acc += 0b11
        end
        acc
      }
    }
    SQ_PX.times { youngs.concat translated_row }

    translated_row = row.each_slice(8 / SQ_PX).map { |slice|
      slice.inject(0) { |acc,i|
        acc <<= 2
        if i == 2
          acc += 0b11
        end
        acc
      }
    }
    SQ_PX.times { olds.concat translated_row }
  end
  rasters =  youngs.flatten.pack('C*')
  oldies =  olds.flatten.pack('C*')
  BitMap.new(columns.length * SQ_PX, max_page_size * SQ_PX, rasters, oldies)
end

$bitmap = BitMap.new 0, 0, '', ''

def init
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1)
  glClearColor(0.0, 0.0, 0.0, 0.0)
end

display = Proc.new do
  glClear(GL_COLOR_BUFFER_BIT)
  glColor3f(0, 1.0, 0)
  glRasterPos2i(0, 0)
  glBitmap($bitmap.width, $bitmap.height, 0.0, 0.0, 0.0, 0.0, $bitmap.young)
  glColor3f(1.0, 0.0, 0)
  glRasterPos2i(0, 0)
  glBitmap($bitmap.width, $bitmap.height, 0.0, 0.0, 0.0, 0.0, $bitmap.oldies)
  glutSwapBuffers()
end

reshape = Proc.new do |w, h|
  glViewport(0, 0, w,  h)
  glMatrixMode(GL_PROJECTION)
  glLoadIdentity()
  glOrtho(0, w, 0, h, -1.0, 1.0)
  glMatrixMode(GL_MODELVIEW)
end

queue = Queue.new

Thread.new do
  server = UNIXServer.new("/tmp/sock")
  sock = server.accept
  puts "CONNECTED"
  while z = sock.readline
    heap = JSON.parse z
    $bitmap = make_bitmap heap
    queue << :updated
  end
end

# Main Loop
# Open window with initial window size, title bar,
# RGBA display mode, and handle input events.
glutInit
glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB)
glutInitWindowSize(200, 820)
glutInitWindowPosition(100, 100)
glutCreateWindow($0)
init()
glutReshapeFunc(reshape)
glutDisplayFunc(display)
glutIdleFunc(lambda {
  queue.pop
  glutPostRedisplay
})
glutMainLoop()
