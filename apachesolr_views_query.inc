<?php

// $Id$ 

// TODO: Spellchecker field? What todo about spellchecker...

/**
 * Class for handling a view that gets its data not from the database, but from
 * a Solr server.
 */
class apachesolr_views_query extends views_plugin_query implements Drupal_Solr_Query_Interface {
  
  /**
   * Static shared by all instances, used to increment ID numbers.
   */
  protected static $idCount = 0;

  /**
   * Each query/subquery will have a unique ID
   */
  public $id;
  
  // facets for the solr query
  protected $_facets = array();
  
  // String containing the query that will be handed to Solr.
  protected $_query;
  
  // Object encapsulating the actual query to Solr.
  protected $_solr_service;
  
  // Array containing the parameters that will be handed to Solr.
  protected $_params = array(
    'facet.mincount' => 1,
    'facet.sort' => 'true'
  );
  // array all of the fields for the query
  protected $_used_fields = array(
    'id',
    'nid',
    'url',
    'uid', // TODO: skip this probably, but not for user searching...
  );
  
  // the template to use on the solr
  protected $_query_template = '';
  
  // the view object. used for the get_path() method
  // @see get_path()
  protected $_view_arguments = array();
  
  // from the view object used in the get_url_queryvalues() method
  // @see get_url_queryvalues()
  protected $_view_filters = array();
  
  // this stores the base path for the view. we store it here as the
  // $view object isn't always avaible
  protected $_base_path = '';
  
  // used to store subqueries. Useful in say the node_access implementation
  protected $_subqueries = array();
  
  // array to store extra url query params. Used for instance to store
  // latitudes and longitudes
  protected $_extra_query = array();
  
  // array to store the sorts for get_available_sorts() call
  protected $_available_sorts = array();
  
  // string to identify the current Views display
  protected $_current_display = '';
  
  // string to identify the current Views name
  protected $_current_views_name = '';
  
  // Array of query fields
  protected $_query_fields = array();
  
  // array of boost functions
  protected $_boost_functions = array();
  
  // array of boost queries
  protected $_boost_queries = array();
  
  protected $base_table = 'apachesolr';
  
  protected $base_field = 'nid';
    
  /**
   * Views init() function
   */
  public function init($base_table, $base_field) {
    $this->base_table = $base_table;  // Predefine these above, for clarity.
    $this->base_field = $base_field;
    $this->_solr_service = apachesolr_get_solr();
    $this->id = ++self::$idCount;
  }
  
  /**
   * Constructor build up the avialable sorts
   */
  public function __construct() {
    $data = views_fetch_data('apachesolr');
    foreach ($data as $field_name => $field) {
      if (!empty($field['sort'])) {
        $this->_available_sorts[$field_name] = array(
          'name' => $field['title'],
          'default' => 'asc', // TODO: make this part of the handler def? So it will be in the Views data defintion
        );
      }
    }
  }
  
  /**
   * copy constructor... what else to do?
   */
  public function __clone() {
    $this->id = ++self::$idCount;
  }
  
  /**
   * Build the query object. Load up all avaivable facets so the blocks work.
   */
  public function build() {
    // Build in the default qf, bf and bq if they haven't been set
    // TODO: remove this when we expose and interface via Views UI
    if (empty($this->_query_template) || $this->_query_template == 'dismax') {
      // if no query fields are set, use default
      // TODO: remove when we have this exposed to the Views UI
      if (empty($this->_query_fields)) {
        $qf = variable_get('apachesolr_search_query_fields', array());
        foreach ($qf as $field_name => $boost) {
            if (!empty($boost)) {
            // if its a normed field
            if ($field_name == 'body') {
              $boost *= 40.0;
            }
            $this->add_query_field($field_name, $boost);
          }
        }
      }
      
      // now we do boost functions
      if (empty($this->_boost_functions)) {
        $this->_params['bf'] = array();
        $solr = apachesolr_get_solr();
        $total = 0;
        if (isset($solr)) {
          $data = $solr->getLuke();
          if (isset($data) && isset($data->index->numDocs)) {
            $total = $data->index->numDocs;
          }
        }
        
        if (empty($total)) {
          $total = db_result(db_query("SELECT COUNT(nid) FROM {node}"));
        }
        
        // date_settings
        $date_settings = variable_get('apachesolr_search_date_boost', '4:200.0');
        list($date_steepness, $date_boost) = explode(':', $date_settings);
        if (!empty($date_boost)) {
          $values = array($date_steepness, $total, $total, $date_boost);
          $this->add_boost_function("recip(rord(created),%f,%d,%d)^%f", $values);
        }
        // comment_settings
        $comment_settings = variable_get('apachesolr_search_comment_boost', '0:0');
        list($comment_steepness, $comment_boost) = explode(':', $comment_settings);
        if ($comment_boost) {
          $values = array($comment_steepness, $total, $total, $comment_boost);
          $this->add_boost_function("recip(rord(comment_count),%f,%d,%d)^%f", $values);
        }
        // changed_settings
        $changed_settings = variable_get('apachesolr_search_changed_boost', '0:0');
        list($changed_steepness, $changed_boost) = explode(':', $changed_settings);
        if ($changed_boost) {
          $values = array($changed_steepness, $total, $total, $changed_boost);
          $this->add_boost_function("recip(rord(last_comment_or_change),%f,%d,%d)^%f", $values);
        }
      }
      // now we do boost queries
      if (empty($this->_boost_queries)) {
        // Boost for nodes with sticky bit set.
        $sticky_boost = variable_get('apachesolr_search_sticky_boost', 0);
        if ($sticky_boost) {
          $this->add_boost_query('sticky', 'true', $sticky_boost);
        }
        // Boost for nodes with promoted bit set.
        $promote_boost = variable_get('apachesolr_search_promote_boost', 0);
        if ($promote_boost) {
          $this->add_boost_query('promote', 'true', $promote_boost);
        }
        // Modify the weight of results according to the node types.
        $type_boosts = variable_get('apachesolr_search_type_boosts', array());
        if (!empty($type_boosts)) {
          foreach ($type_boosts as $type => $boost) {
            // Only add a param if the boost is != 0 (i.e. > "Normal").
            if (!empty($boost)) {
              $this->add_boost_query('type', $type, $boost);
            }
          }
        }
      }
    }
    try {
      // TODO: add in config screen on teh View instead of using apachesolr settings
      // here we add in all the facet stuff for things that are turned on
      // we rely on the apachesolr_search module to provide them for us
      apachesolr_search_add_facet_params($this->_params, $this);
      if ($this->_query_template == 'dismax' || empty($this->_query_template)) {
        // add in the qf to the params array here
        $this->_params['qf'] = array();
        foreach ($this->_query_fields as $field_name => $boost) {
          $this->_params['qf'][] = "$field_name^$boost";
        }
        
        // add in the bf
        $this->_params['bf'] = $this->_boost_functions;
        
        // build the bq
        foreach ($this->_boost_queries as $field_name => $fields) {
          foreach ($fields as $field_value => $boost) {
            $this->_params['bq'][] = "$field_name:$field_value^$boost";
          }
        }
      }
      
      // Add in facets now
      $this->_params['fq'] = $this->rebuild_fq();
      
      
      // process subqueries
      foreach ($this->_subqueries as $query_data) {
        // process the query string first
        $sub_query_string = $query_data['query']->get_query_basic();
        if ($sub_query_string) {
          $this->_query .= " {$query_data['q_op']} ({$sub_query_string})";
        }
        
        // now handle the filter query
        $subfq = $query_data['query']->get_fq();
        if (!empty($subfq)) {
          $operator = $query_data['fq_op'];
          $this->_params['fq'][] = "(" . implode(" {$operator} ", $subfq) .")";
        }
      }
      
      // add in the fields. These fields include the necessary fields.
      $this->_params['fl'] = implode(',', $this->_used_fields);
      
      // query template handling
      if (!empty($this->_query_template)) {
        $this->_params['qt'] = $this->_query_template;
      }
    }
    catch (Exception $e) {
      watchdog('apachesolr_views', $e->getMessage());
    }
  }
  
  /**
   * Let modules modify the query just prior to finalizing it.
   */
  public function alter(&$view) {
    foreach (module_implements('apachesolr_modify_query') as $module) {
      $function = $module . '_apachesolr_modify_query';
      $function($this, $this->_params, 'apachesolr_views_query');
    }
  }
  
  /**
   * Executes the query and fills the associated view object with according
   * values.
   * 
   * Values to set: $view->result, $view->total_rows, $view->execute_time,
   * $view->pager['current_page'].
   */
  public function execute(&$view) {
    $start_time = views_microtime();
    $view->result = array();
    $this->_view_arguments = $view->argument;
    $this->_view_filters = $view->filter;
    $this->_base_path = isset($view->display[$view->current_display]->display_options['path']) ? $view->display[$view->current_display]->display_options['path'] : '';
    $this->_current_display = $view->current_display;
    $this->_current_views_name = $view->name;

    try {
      // TODO: handle multiple pages seperated by commas. @see views_plugin_query_default.inc
      $page = isset($_GET['page']) ? $_GET['page'] : 0;
      $offset = $page * $view->pager['items_per_page'];
      
      $response = $this->_solr_service->search($this->_query, $offset, $view->pager['items_per_page'], $this->_params);
      
      $view->total_rows = $total = $response->response->numFound;
      
      // we do a fake pager here. Set the globals so the pager works (tm)
      if (!empty($view->pager['use_pager'])) {
        $view->pager['current_page'] = $offset / $view->pager['items_per_page'];
        
        // OOO! globals. This is to make paging work..
        global $pager_page_array, $pager_total, $pager_total_items;
        
        // Set the item count for the pager.
        $pager_total_items[$view->pager['element']] = $view->total_rows;
        
        // Calculate and set the count of available pages.
        $pager_total[$view->pager['element']] = ceil($pager_total_items[$view->pager['element']] / $view->pager['items_per_page']);

        // What page was requested:
        // TODO: handle multiple page requests seperated by commas.
        $pager_page_array = array($page);
        $pager_page_array[$view->pager['element']] = $view->pager['current_page'];
      }
      
      // The response is cached so that it is accessible to the blocks and
      // anything else that needs it beyond the initial search.
      // TODO: allow this to be configurable on the Views UI
      if ($view->display[$view->current_display]->display_plugin != 'block') {
        apachesolr_static_response_cache($response);
        apachesolr_has_searched(TRUE);
        apachesolr_current_query($this);
      }
      
      if ($total > 0) {
        $results = $response->response->docs;
        
        // Process dates
        $date_fields = array('created', 'changed');
        foreach (array_values($date_fields) as $field) {
          if (empty($this->_used_fields[$field])) {
            unset($date_fields[$field]);
          }
        }
        if (!empty($date_fields)) {
          foreach ($results as $doc) {
            foreach ($date_fields as $field) {
              $doc->$field = strtotime($doc->$field);
            }
          }
        }
        
        $view->result = $results;
      }
    }
    catch (Exception $e) {
      watchdog('Apache Solr', $e->getMessage(), NULL, WATCHDOG_ERROR);
      apachesolr_failure(t('Solr search'), is_null($query) ? $this->_keys : $query->get_query_basic());
    }
    
    $view->execute_time = views_microtime() - $start_time;
  }
  
  
  /**
   * Add a field to retrieve.
   * 
   * $compat_field is used for compatibility with the views_plugin_query_default
   * definition of this method - when $compat_field is set, $field is ignored
   * and $compat_field used instead.
   */
  public function add_field($field, $compat_field = NULL) {
    if (!empty($compat_field)) {
      $field = $compat_field;
    }
    if (is_array($field) && isset($field['field'])) {
      $field = $field['field'];
    }
    if (empty($field) || !is_string($field)) {
      return FALSE;
    }
    if (!in_array($field, $this->_used_fields)) {
      $this->_used_fields[] = $field;
    }
    return $field;
  }
  
  /**
   * Add a sorting directive.
   * 
   * @param $single If TRUE, the results will only be sorted by this order.
   */
  public function add_sort($field, $order, $single = FALSE) {
    $sort = $field . ' ' . strtolower($order);
    if (empty($this->_params['sort']) || $single) {
      $this->_params['sort'] = $sort;
    }
    else {
      $this->_params['sort'] .= ',' . $sort;
    }
  }
  
  /**
   * get a search param. Used primarly by the snippet field
   *
   * @param string $param_name
   *
   * @return string parameter setting
   */
  function get_param($param_name) {
    return $this->_params[$param_name];
  }
  
  /**
   * set the search param. useful for extensions like localsolr
   *
   * @param string $param_name
   * name of the parameter
   *
   * @param string $param_value
   * value of the parameter
   *
   * @return none;
   */
  function set_param($param_name, $param_value) {
    $this->_params[$param_name] = $param_value;
  }
  
  /**
   * Add in a facet string
   *
   * @param string $type
   * The type of facet. use apachesolr_index_key() for dynamic fields
   *
   * @param string $value
   * The value of the facet. Can be "story OR page" to filter multiple
   *
   * @param boolean $exclude
   * Whether or not to exclude the value from the results
   *
   * @return none
   */
  public function add_filter($type, $value, $exclude = FALSE) {
    $this->_facets[$type][] = array(
      'value' => $value,
      'exclude' => $exclude,
    );
  }
  
  /**
   * This function sets the query string
   *
   * @param string $query
   * plain text typed in search query
   *
   * @return none
   */
  public function set_query($query) {
    return $this->_query = $query;
  }
  
  /**
   * return the basic query
   */
  public function get_query_basic() {
    return $this->_query;
  }
  
  /**
   * Checks to see if the facet has bene applied
   *
   * @param string $field
   * the facet field to check
   *
   * @param string $value
   * The facet value to check against
   */
  function has_filter($field, $value) {
    if (isset($this->_facets[$field])) {
      foreach ($this->_facets[$field] as $definition) {
        if ($definition['value'] == $value && $definition['exclude'] == FALSE) {
          return TRUE;
        }
      }
    }
    
    return FALSE;
  }
  
  /**
   * Remove a facet from the query
   *
   * @param string $field
   * the facet field to remove
   *
   * @param string $value
   * The facet value to remove
   * This value can be NULL
   */
  function remove_filter($field, $value = NULL) {
    if (!empty($value)) {
      if (isset($this->_facets[$field])) {
        $removal_key = FALSE;
        foreach ($this->_facets[$field] as $key => $definition) {
          if ($definition['value'] == $value) {
            $removal_key = $key;
            break;
          }
        }
        // we found it delete the value
        if ($removal_key !== FALSE) {
          unset($this->_facets[$field][$removal_key]);
        }
      }
    }
    else if (isset($this->_facets[$field])) {
      unset($this->_facets[$field]);
    }
  }
  
  /**
   * return the search path
   */
  function get_path() {
    $wildcard_count = 0; // this is used to remove the some/path/argument/all/all paths
    if (empty($this->_view_arguments)) {
      return $this->_base_path;
    }
    foreach ($this->_view_arguments as $field => $argument) {
      // because some arguments arn't standard base arguments we need to do things differently
      // so that the facet blocks behave. So any argument thats not part of a facet block
      // we have its value copied into the path
      // arguments that are a facet block argument will be processed differently
      $path_part = $this->part_of_facet_block($argument) ? $this->argument_part($field) : $argument->argument;
      if (!empty($path_part)) {
        $path_parts[$argument->position + 1] = $path_part;
        $wildcard_count = 0;
      }
      else {
        $path_parts[$argument->position + 1] = $argument->options['wildcard'];
        $wildcard_count++;
      }
    }
    $arguments = explode('/', $this->_base_path);
    $path = '';
    foreach ($arguments as $arg) {
      if ($arg == '%') {
        $part = array_shift($path_parts);
      }
      else {
        $part = $arg;
      }
      $path .= "/$part";
    }
    
    if (count($path_parts) && $wildcard_count != count($path_parts)) {
      $path .= "/" .  implode('/', array_slice($path_parts, 0, count($path_parts) - $wildcard_count));
    }
    $path = substr($path, 1);
    if (trim($path) == trim(variable_get('site_frontpage', 'node'))) {
      return '<front>';
    }
    return $path;
  }
  
  /**
   * return the facet argument suitable for Views
   *
   * @param string $field
   * name of the facet
   *
   * @return string the part of the url for Views that matches this facet value
   */
  protected function argument_part($field) {
    $argument_path = '';
    if (empty($this->_facets[$field])) {
      return '';
    }
    foreach ($this->_facets[$field] as $defintion) {
      if (!$defintion['exclude']) {
        $argument_path .= ',' . $defintion['value'];
      }
    }
    
    return drupal_substr($argument_path, 1);
  }
  
  /**
   * determine if argument field is part of a facet block
   *
   * @return bool
   * Whether or not the provided argument could be in a facet block
   */
  protected function part_of_facet_block($argument) {
    $facet_arguments = array(
      'tid',
      'type',
      'uid',
    );
    // we use field instead of real_field
    // because we want these specific ones defined in
    // apachesolr_views.views.inc
    return (in_array($argument->field, $facet_arguments));
  }
    
  /**
   * return any query string for use in the l function
   *
   * @see l()
   */
  function get_url_queryvalues() {
    // goes through and finds all the exposed filters
    $query_values = array();
    foreach ($this->_view_filters as $filter) {
      if ($filter->options['exposed']) {
        if ($filter->field == 'text' && !empty($this->_query)) {
          // the search exposed filter is special
          $query_values[$filter->options['expose']['identifier']] = $this->_query;
        }
        elseif ($this->_facets[$filter->field]) {
          $query_values[$filter->options['expose']['identifier']] = $this->_facets[$filter->field];
        }
        elseif (!empty($_GET[$filter->options['expose']['identifier']])) {
          $query_values[$filter->options['expose']['identifier']] = $_GET[$filter->options['expose']['identifier']];
        }
      }
    }
    
    foreach ($this->_extra_query as $field => $value) {
      $query_values[$field] = $value;
    }
    
    return $query_values;
  }
  
  /**
   * set the solrsort. This currently doesn't work
   *
   * @param string $field
   * the field to sort on
   *
   * @param string $direction
   * the direction in which to sort (ASC/DESC)
   */
  function set_solrsort($field, $direction) {
    $this->add_sort($field, $direction);
  }

  /**
   * Get the solrsort
   *
   * @return array
   * array keyed with #name and #direction for every sort applied to this query
   */
   function get_solrsort() {
     return $this->_params['sort'];
   }
  
  /**
   * return an array of filters
   *
   * @param string $name
   * the name of the filter applied to the query
   * 
   * Compabatiablity layer with Solr_Base_Query
   */
  function get_filters( $name = NULL) {
    $filters = array();
    foreach ($this->_facets as $type => $fields) {
      foreach ($fields as $data) {
        $filters[] = array(
          '#name' => ($data['exclude']) ? "NOT $type" : $type, 
          '#value' => $data['value']
        );
      }
    }
    
    // if we are looking for a specific one
    // remove all those that don't match and return the array
    if (!empty($name)) {
      foreach ($filters as $key => $filter) {
        if ($filter['#name'] != $name) {
          unset($filters[$key]);
        }
      }
    }
    
    return $filters;
  }
  
  function get_fq() {
    return $this->rebuild_fq();
  }
  
  /**
   * return the Views display id
   */
  function get_display_id() {
    return $this->_current_display;
  }
  
  /**
   * return the Views name
   */
  function get_views_name() {
    return $this->_current_views_name;
  }
  
  /**
   * add a subquery to the current query
   */
  function add_subquery(Drupal_Solr_Query_Interface $query, $fq_operator = 'OR', $q_operator = 'AND') {
    $this->_subqueries[$query->id] = array(
      'query' => $query,
      'fq_op' => $fq_operator,
      'q_op' => $q_operator,
    );
  }
  /**
   * Escapes a term for passing it to the query.
   */
  public static function escape_term($term) {
    $term = trim($term);
    if (empty($term)) {
      return '';
    }
    if (($term{0} == '"' && $term{strlen($term)-1} == '"')
        || $term{0} == '(' && $term{strlen($term)-1} == ')') {
      return $term;
    }
    
    if (strpos($term, ' ') !== FALSE) {
      return Drupal_Apache_Solr_Service::phrase($term);
    }
    return Drupal_Apache_Solr_Service::escape($term);
  }
  
  /**
   * allows the user to change the query template
   * @see localsolr
   *
   * @param string $qt
   * the query template name to use
   */
  public function change_query_template($qt) {
    $this->_query_template = $qt;
  }
  
  /**
   * add a field and value to the query string
   * This is useful for generating links with the facet blocks
   *
   * @param string $field
   * the query field
   *
   * @param string $value
   * the value of hte query field
   */
  function add_url_query_param($field, $value) {
    $this->_extra_query[$field] = $value;
  }
  
  public function get_available_sorts() {
    return $this->_available_sorts;
  }
  
  /**
   * Remove a specific subquery
   *
   * @param Drupal_Solr_Query_Interface $query
   * the query to remove
   */
  function remove_subquery(Drupal_Solr_Query_Interface $query) {
    unset($this->_subqueries[$query->id]);
  }
  
  /**
   * remove all subqueries
   */
  function remove_subqueries() {
    $this->_subqueries = array();    
  }
  
  /**
   * make a sort available
   */
  function set_available_sort($field, $sort) {
    // TODO: hmm.. wierd... we pull in all the sorts from Views data fields
  }
  
  /**
   * adds query fields to the query with a specified weight
   *
   * @param string $field_name
   * name of the field in the schema
   *
   * @param double $boost
   * the amount of boost applied
   */
  function add_query_field($field_name, $boost = 1.0) {
    $this->_query_fields[$field_name] = $boost;
  }
  
  /**
   * remove a query field
   *
   * @param $field_name
   * the name of the field in the schema
   */
  function remove_query_field($field_name) {
    unset($this->_query_fields[$field_name]);
  }
  
  /**
   * adds a query boost function to the query
   *
   * @param string $function
   * String with placeholders can use one or more of: http://wiki.apache.org/solr/FunctionQuery
   *
   * @param array $values
   * the values to be sprintf'd in place
   */
  function add_boost_function($function, array $values) {
    $this->_boost_functions[] = vsprintf($function, $values);
  }
  
  /**
   * adds a query boost for a specific field/value combo
   *
   * @param string $field_name
   * name of field in the schema
   *
   * @param string $field_value
   * the value of the field to boost
   *
   * @param double $boost_value
   * the amount of boost to apply
   */
  function add_boost_query($field_name, $field_value, $boost_value) {
    $this->_boost_queries[$field_name][$field_value] = $boost_value;
  }
  
  /**
   * remove boost query
   *
   * @param string $field_name
   * the name of the field in the schema
   *
   * @param string $field_value
   * The value of the field we are boosting. Can be NULL to remove all boosts on $field_name
   */
  function remove_boost_query($field_name, $field_value = NULL) {
    if ($field_value) {
      unset($this->_boost_queries[$field_name][$field_value]);
    }
    else {
      unset($this->_boost_queries[$field_name]);
    }
  }
  
  /**
   * build up the filter query params that are passed to Solr
   */
  protected function rebuild_fq() {
    $query_filters = array();
    foreach ($this->_facets as $type => $facets) {
      $filter_string = '';
      foreach ($facets as $definition) {
        if ($definition['exclude']) {
          $type_string = "NOT $type";
        }
        else {
          $type_string = $type;
        }
        $query_filters[] = "$type_string:" . $definition['value'];
      }
    }
    
    return $query_filters;
  }
}