<script lang="ts">

  async function getData() {
    let response = await fetch("/api/proj-data");
    let users = await response.json();
    return users;
  }
  const promise = getData();
</script>

<div class="bg-indigo-300 object-cover flex-col p-8 text-center ">

  <h1 class="text-xl" >proj data test page</h1>

</div>

<div class="bg-indigo-300 object-cover flex-col p-8 text-center ">
  <h2>TIME:</h2>
  <h2 class="font-mono text-6xl">

    <div>
      {#await promise}
        <p>Loading...</p>
      {:then users}
        <p>{users.temperature_str}, {users.time_str}</p>
      {:catch error}
        <p style="color: red">{error.message}</p>
      {/await}
      </div>

    <!-- <span style="{`${time_str}`}">
      {`${time_str}`}
    </span> -->
  </h2> 
</div>

<div class="bg-indigo-300 object-cover flex-col p-8 text-center ">
<h2>TEMPERATURE:</h2>
<h2 class="font-mono text-6xl">
  <!-- <span style="{`${temper_str}`}">
    {`${temper_str}`}
  </span> -->
</h2>
</div>

