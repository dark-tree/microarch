python
import gdb

class SourceSpanPrinter:
	def __init__(self, val):
		self.val = val

	def to_string(self):
		begin = int(self.val['begin'])
		end = int(self.val['end'])
		size = end - begin
		if size <= 0 or size > 4096:
			return "<invalid>"

		# gdb.Value.string() reads a char* as C-string; with length we can force substring
		return '"' + self.val['begin'].string(length=size) + '"'

	def children(self):
		yield "begin", self.val['begin']
		yield "end", self.val['end']

class TokenPrinter:
	def __init__(self, val):
		self.val = val

	def to_string(self):
		m_type = str(self.val['m_type'])
		m_span = gdb.default_visualizer(self.val['m_span'])

		return m_type + ": " + m_span.to_string()

	def children(self):
		yield "m_type", self.val['m_type']
		yield "m_span", self.val['m_span']

def build_pretty_printers():
	pp = gdb.printing.RegexpCollectionPrettyPrinter("xenon")
	pp.add_printer('SourceSpan', '^SourceSpan$', SourceSpanPrinter)
	pp.add_printer('Token', '^Token$', TokenPrinter)
	return pp

gdb.printing.register_pretty_printer(gdb.current_objfile(), build_pretty_printers())
end