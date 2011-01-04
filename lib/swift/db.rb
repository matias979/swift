module Swift
  module DB
    class Mysql < Adapter
      def initialize options = {}
        super options.update(driver: 'mysql')
      end

      def returning?
        false
      end
    end # Mysql

    class Sqlite3 < Adapter
      def initialize options = {}
        super options.update(driver: 'sqlite3')
      end

      def returning?
        false
      end

      def migrate! scheme
        keys   =  scheme.header.keys
        serial =  scheme.header.find(&:serial)
        fields =  scheme.header.map{|p| field_definition(p)}.join(', ')
        fields += ", primary key (#{keys.join(', ')})" unless serial or keys.empty?

        execute("drop table if exists #{scheme.store}")
        execute("create table #{scheme.store} (#{fields})")
      end

      def field_type attribute
        case attribute
          when Type::String     then 'text'
          when Type::Integer    then attribute.serial ? 'integer primary key' : 'integer'
          when Type::Float      then 'float'
          when Type::BigDecimal then 'numeric'
          when Type::Time       then 'timestamp'
          when Type::Date       then 'date'
          when Type::Boolean    then 'boolean'
          when Type::IO         then 'blob'
          else 'text'
        end
      end
    end # Sqlite3

    class Postgres < Adapter
      def initialize options = {}
        super options.update(driver: 'postgresql')
      end

      def returning?
        true
      end

      def field_type attribute
        case attribute
          when Type::IO then 'bytea'
          else super
        end
      end
    end # Postgres
  end # DB
end # Swift
